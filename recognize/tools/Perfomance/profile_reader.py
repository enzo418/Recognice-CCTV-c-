from typing import NamedTuple # >= py 3.6

class Profile(NamedTuple):
    build: str
    executable: str
    executable_args: str
    samples_memory: int
    samples_cpu: int
    functions: list[str]

class UknownHeader(Exception):
    pass


class MissingHeaderException(Exception):
    pass

def read_profile(path: str) -> Profile:
    """Reads a profile (configuration)

    Args:
        path (str): path of the configuration file

    Raises:
        MissingHeaderException: When a required header is missing in the file

    Returns:
        Profile: Profile readed
    """
    headers = {}
    headers_required = ["build", "executable", "samples_cpu", "samples_memory", "functions", "executable_args"]

    with open(path, 'r', encoding='utf-8') as file:
        for line in file:
            line = line.strip()

            # min string is a=b
            if len(line) > 3:
                splitted = line.split("=", 1)
                if len(splitted) == 2:
                    # unpack the strings from the list
                    header, value = splitted

                    # add the header to the dictionary
                    headers[header] = value

        # remove the headers found
        for header in headers:
            if header in headers_required:
                headers_required.remove(header)

        # check for missing headers
        if len(headers_required) != 0:
            raise MissingHeaderException(', '.join(headers_required))

        profile = Profile(
            build=headers["build"],
            executable=headers["executable"],
            samples_memory=int(headers["samples_memory"]),
            samples_cpu=int(headers["samples_cpu"]),
            functions=_process_functions_header(headers["functions"]),
            executable_args=headers["executable_args"]
        )

        return profile

def _process_functions_header(value:str)->list[str]:
    return value.split(",")
