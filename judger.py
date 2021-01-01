import json
import subprocess

judge_path = "./judge"


def judge(proc_args):
    proc = subprocess.Popen(proc_args, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    out, err = proc.communicate()
    if err:
        raise ValueError("Error occurred while calling judge: {}".format(err))

    return json.loads(out.decode("utf-8"))


proc_args = [
    judge_path,
    "judge",
    "./main",
    "1000",
    "2048",
    "./tests/1/1.in",
    "./tests/1/1.out",
    "1.tmp.out",
    "-l",
    "1.log",
]

result = judge(proc_args)
print("result: ", result)

proc_args = [
    judge_path,
    "run",
    "./main",
    "1000",
    "2048",
    "./tests/1/1.in",
    "1.tmp.out",
    "-l",
    "1.log",
]
result = judge(proc_args)
print("result: ", result)


proc_args = [
    judge_path,
    "check",
    "./tests/1/1.out",
    "1.tmp.out",
    "-l",
    "1.log",
]
result = judge(proc_args)
print("result: ", result)
