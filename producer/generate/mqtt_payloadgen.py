import sys
import json

STDOUT = None
IMPORTS = "imports"
DIRECTIVES = "directives"
NUM = "num"


def raise_value_error(message: str):
    raise ValueError(message)


def parse_json_template(template: str) -> dict:
    return json.loads(template)


def import_dynamically(import_list: list[str]):
    for symbol in import_list:
        globals()[symbol] = __import__(symbol)


def generate_payloads(directives: dict, num: int) -> list[dict]:
    return [{k: eval(v, globals()) for k, v in directives.items()} for _ in range(num)]


def dump_payloads(payloads: list[dict], file=STDOUT, wmode="w"):
    file = open(file, wmode) if file is not None else STDOUT
    print(json.dumps(payloads), file=file)
    _ = file.close() if file is not None else None


def exec_payload_generate(raw_template: dict, num=100, output=STDOUT, wmode="w"):
    parsed_template = parse_json_template(raw_template)
    _ = import_dynamically(
        parsed_template[IMPORTS]) if IMPORTS in parsed_template else None
    payloads = generate_payloads(
        parsed_template[DIRECTIVES], num) if DIRECTIVES in parsed_template else raise_value_error("No directives given")
    dump_payloads(payloads, output, wmode)
    print(f"Payloads dumped to: {output if output is not None else 'stdout'}")


if __name__ == "__main__":
    scriptname = sys.argv[0]
    scriptargs = sys.argv[1:]

    input_file = "ARGV"
    output_file = "STDOUT"
    num_generate = 100
    skip = -1
    wmode = "w"
    for arg in scriptargs:
        if arg.startswith("--in-file") or arg.startswith("-i"):
            input_file = arg[arg.index("=") + 1:]
            skip += 1
        elif arg.startswith("--out-file") or arg.startswith("-o"):
            output_file = arg[arg.index("=") + 1:]
            skip += 1
        elif arg.startswith("--num-generate") or arg.startswith("-n"):
            num_generate = int(arg[arg.index("=") + 1:])
            skip += 1
        elif arg.startswith("--binary") or arg.startswith("-b"):
            wmode = "wb"
            skip += 1
        else:
            if input_file != "ARGV":
                raise ValueError(f"Invalid argument: {arg}")

    if input_file == "ARGV":
        template = "".join(scriptargs[skip + 1:])
    else:
        ifile = open(input_file, "r")
        template = ifile.read()
        ifile.close()

    assert len(template) > 0, "Length of the template is too small"
    exec_payload_generate(template, num_generate, eval(output_file), wmode)
