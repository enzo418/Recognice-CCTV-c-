# Run valgrind to reads all the calls (callgrind)
# Run valgrind to read all the memory allocations (with leak-check)
# Get the relevant information for each one
# Run it several times to build a signifincant sample
# Show the average of every important data collected

# Input: the path to a file that has this three lines:
# build=g++ ~/.local/share/kdevscratchpad/scratches/Opencv_ImageManipulation.cpp `pkg-config --cflags opencv` -std=c++20 -o /tmp/a.out $f `pkg-config --libs opencv` -lpthread
# executable=/tmp/a.out
# samples=10

# e.g. python3 GetPerfomanceInformation.py /tmp/profile.txt

import logging
from profile_reader import Profile, read_profile, MissingHeaderException
import sys
import os
import subprocess
import traceback

import perfomance_runner as perf

logging.basicConfig(
    filename='perfomance.log',
    level=logging.INFO,
    format='%(asctime)s %(levelname)-8s %(message)s',
    datefmt='%Y-%m-%d %H:%M:%S'
)

log = logging.getLogger()

log.info("-------------------")

def start():
    if len(sys.argv) == 0:
        print("No file given")
        sys.exit(0)

    file_path = sys.argv[1]

    log.info(f"Script args: {str(sys.argv)}")

    if not os.path.isfile(file_path):
        print("Path given is not a file or doesn't exists.")
        sys.exit(0)

    profile = None

    try:
        profile = read_profile(file_path)
    except MissingHeaderException as error:
        print("Missing the following header(s) in the configuration file: ", str(error))
        sys.exit(0)

    # sucess, output = run_build(profile)
    sucess = True

    if sucess:
        run_perfomance_tests(profile)
    else:
        print("ERROR: build failed")
        # print(output)
        sys.exit(0)


def run_perfomance_tests(profile: Profile):
    # TODO: 1. Apply multithreading
    #       2. Add cpu time test
    #       99. Better error handling

    cpu_results = []
    for n in range(profile.samples_cpu):
        log.info(f"Running cpu perfomance test number {n}")

        try:
            cpu_results.append(
                perf.run_cpu_perfomance(
                    profile.executable, profile.executable_args, profile.functions
                )
            )
        except Exception as error:
            print("Couldn't get the CPU results, error: ", str(error))
            print(traceback.format_exc())

    if len(cpu_results) > 0:
        cpu_result = cpu_results[0]
        for i in range(1,len(cpu_results)):
            cpu_result = cpu_result + cpu_results[i]
        
        cpu_result = cpu_result / len(cpu_results)

        print("===== CPU RESULTS ===== ")
        for result in cpu_result.results:
            print("- ", result.name,":")
            print("\t IR: ", result.ir_per_call)
            print("\t Call count: ", result.call_count if result.call_count is not None else "Uknown")
            print("\t IR %: ", result.percentage)
            print()
    
    # memory

    memory_results = []
    for n in range(profile.samples_cpu):
        log.info(f"Running memory perfomance test number {n}")
        try:
            memory_results.append(
                perf.run_memory_perfomance(
                    profile.executable, profile.executable_args)
            )
        except Exception as error:
            print("Couldn't get the MEMORY results, error: ", str(error))
            print(traceback.format_exc())
    
    if len(memory_results) > 0:
        memory_result = memory_results[0]
        for i in range(1,len(memory_results)):
            memory_result = memory_result + memory_results[i]
        
        memory_result = memory_result / len(memory_results)

        print("===== MEMORY RESULTS ===== ")
        print("Allocs count: ", memory_result.total_allocs)
        print("Frees count: ", memory_result.total_frees)
        print("Bytes allocated: ", memory_result.total_bytes,
            f" ({memory_result.total_bytes / 10**6} MB)")


def run_build(profile: Profile) -> tuple[bool, str]:
    compiler, args = profile.build.split(" ", 1)
    result = subprocess.run([compiler, args], check=False, text=True)
    sucess = result.returncode == 0

    return sucess, result.stdout

start()