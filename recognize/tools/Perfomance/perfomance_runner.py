from os import system
import subprocess
import re

from typing import NamedTuple  # >= py 3.6

import logging

log = logging.getLogger()


class FunctionPerfomanceResultCPU(NamedTuple):
    name: str
    ir_per_call: int
    call_count: int
    percentage: float

    def __add__(self, other):
        i_r = self.ir_per_call + other.ir_per_call
        c_c = self.call_count + other.call_count
        p_c = self.percentage + other.percentage

        return FunctionPerfomanceResultCPU(
            name=self.name,
            ir_per_call=i_r,
            call_count=c_c,
            percentage=p_c
        )

    def __truediv__(self, other):
        if isinstance(other, (int, float)):
            i_r = self.ir_per_call / other
            c_c = self.call_count / other
            p_c = self.percentage / other

            return FunctionPerfomanceResultCPU(
                name=self.name,
                ir_per_call=i_r,
                call_count=c_c,
                percentage=p_c
            )
        else:
            i_r = self.ir_per_call / other.ir_per_call
            c_c = self.call_count / other.call_count
            p_c = self.percentage / other.percentage

            return FunctionPerfomanceResultCPU(
                name=self.name,
                ir_per_call=i_r,
                call_count=c_c,
                percentage=p_c
            )


class PerfomanceResultCPU(NamedTuple):
    results: list[FunctionPerfomanceResultCPU]

    def __add__(self, other):
        final_results = []
        for result in self.results:
            for result2 in other.results:
                if result.name == result2.name:
                    final_results.append(result + result2)
                    break

        return PerfomanceResultCPU(
            results=final_results
        )

    def __truediv__(self, other):
        if isinstance(other, (int, float)):
            final_results = []
            for result in self.results:
                final_results.append(result/other)

            return PerfomanceResultCPU(
                results=final_results
            )
        else:
            final_results = []
            for result in self.results:
                for result2 in other.results:
                    if result.name == result2.name:
                        final_results.append(result/result2)
                        break

            return PerfomanceResultCPU(
                results=final_results
            )


class PerfomanceResultMemory(NamedTuple):
    total_allocs: int
    total_frees: int
    total_bytes: int

    def __add__(self, other):
        allocs = self.total_allocs + other.total_allocs
        frees = self.total_frees + other.total_frees
        bytes = self.total_bytes + other.total_bytes
        return PerfomanceResultMemory(
            total_allocs=allocs,
            total_frees=frees,
            total_bytes=bytes
        )

    def __truediv__(self, other):
        if isinstance(other, (int, float)):
            allocs = self.total_allocs / other
            frees = self.total_frees / other
            bytes = self.total_bytes / other
            return PerfomanceResultMemory(
                total_allocs=allocs,
                total_frees=frees,
                total_bytes=bytes
            )
        else:
            allocs = self.total_allocs / other.total_allocs
            frees = self.total_frees / other.total_frees
            bytes = self.total_bytes / other.total_bytes
            return PerfomanceResultMemory(
                total_allocs=allocs,
                total_frees=frees,
                total_bytes=bytes
            )


def run_cpu_perfomance(excutable_path: str, args: str, pFunctions: list[str]) -> PerfomanceResultCPU:
    log.debug("Running callgrind")

    functions = pFunctions.copy()

    result = _run_command(
        ["valgrind", "--tool=callgrind", excutable_path, args])
    sucess = result.returncode == 0

    output = result.stdout

    if not sucess:
        raise Exception("Valgrind had and error: ", output)

    process_id = re.search(r"\={2}(\d+)\={2}", output).group(1)
    callgrind_file = "callgrind.out." + process_id

    log.debug(f"Running callgrind_annotate for {callgrind_file}")

    # run callgrind_annotate
    result = _run_command(
        ["callgrind_annotate", "--inclusive=yes", "--context=0", "--threshold=100", callgrind_file])

    sucess = result.returncode == 0

    output = result.stdout

    if not sucess:
        raise Exception("callgrind_annotate had and error: ", output)

    func_perfomance = {}

    for line in output.splitlines():
        for func in functions:
            if func in line:
                func_perfomance[func] = line

    # ok we now have all the lines where the perfomance of the function is detailed

    # remove the functions found
    for header in func_perfomance:
        if header in functions:
            functions.remove(header)

    # check for missing functions
    if len(functions) != 0:
        log.warning("Warning: The following functions couldn't be found on the results: ".format(
            ", ".join(functions)))
        # print("Warning: The following functions couldn't be found on the results: ",
        #       ", ".join(functions))

    functions_results = []

    for function, line in func_perfomance.items():
        # "3,102,796 ( 0.13%)  => function_signature (Tx)", T=number of calls
        ir_per_call = _valgrind_search_numeric_value(line, "")
        if ir_per_call is not None:
            ir_per_call = _valgrind_parse_string_to_int(ir_per_call.group(1))

        percentage = re.search(r"(\d+\.\d+)\%", line)
        if percentage is not None:
            percentage = float(
                percentage.group(1).replace(",", ".")
            )

        call_count = re.search(r"\((\d+)x\)", line)
        if call_count is not None:
            call_count = call_count.group(1)

        functions_results.append(
            FunctionPerfomanceResultCPU(
                name=function,
                ir_per_call=ir_per_call or -1,
                call_count=call_count or -1,
                percentage=percentage or -1
            )
        )

    return PerfomanceResultCPU(
        results=functions_results
    )


def run_memory_perfomance(excutable_path: str, args: str) -> PerfomanceResultMemory:
    log.debug("Running memory test")
    result = _run_command(
        ["valgrind", "--leak-check=yes", excutable_path, args])
    sucess = result.returncode == 0

    if not sucess:
        raise Exception("Valgrind had and error: ", result.stdout)

    result = result.stdout

    # get the 15 lines after the heap summary
    heap_summary = _get_next_lines(
        result, 15, _count_lines_until(result, "HEAP SUMMARY"))
    values_required = ["allocs", "frees", "bytes allocated"]
    values = {}

    for value in values_required:
        match = _valgrind_search_numeric_value(heap_summary, value)

        if match is None:
            raise Exception(f"Couldn't get the total '{value}' from valgrind")

        values[value] = _valgrind_parse_string_to_int(match.group(1))

    return PerfomanceResultMemory(
        total_allocs=values["allocs"] or -1,
        total_frees=values["frees"] or -1,
        total_bytes=values["bytes allocated"] or -1,
    )


def _run_sync(args: list[str]):
    return


def _valgrind_parse_string_to_int(source: str) -> int:
    return int(source.replace(",", "").replace(".", ""))


def _valgrind_search_numeric_value(source: str, descriptor: str) -> str:
    return re.search(r"(\d+([,.]\d{3})*)\ " + descriptor, source)


def _count_lines_until(source: str, substring: str) -> int:
    count = 0
    for line in source.splitlines():
        if substring not in line:
            count += 1
        else:
            break
    return count


def _get_next_lines(source: str, lines_count: int, start_from_line=0) -> str:
    out = ""
    start = start_from_line
    count = lines_count

    for line in source.splitlines():
        if start == 0 and count > 0:
            out += line
            count -= 1
        else:
            start -= 1

    return out


def _run_command(args: list[str]):
    return subprocess.run(
        args,
        stdout=subprocess.PIPE,
        stderr=subprocess.STDOUT,
        check=False,
        text=True)


def _get_string_from_byte(bytes: any) -> str:
    return bytes.decode("utf-8")
