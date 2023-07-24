import os

directory = "src"
output_path = "include/engine"

def iterate(start, include):
    for filename in os.listdir(start):
        f = os.path.join(start, filename)
        if start == ".": 
            f = filename
        if os.path.isfile(f):
            if os.path.splitext(f)[1] == ".h": 
                os.system("cp " + f + " ../" + output_path + "/" + f)
        if os.path.isdir(f):
            os.system("mkdir " + "../" + output_path + "/" + f)
            iterate(f, include)

def main():
    os.system("rm -rf " + output_path)
    os.system("mkdir " + output_path)
    os.chdir(directory)
    iterate(".", output_path)

main()
