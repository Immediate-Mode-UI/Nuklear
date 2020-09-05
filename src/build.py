
import fnmatch
import os.path
import sys
import re

def print_help():
    print(
"""usage: python single_header_packer.py --macro <macro> [--intro <files>] --extern <files> --pub <files> --priv1 <files> --priv2 <files> [--outro <files>]

       where <files> can be a comma-separated list of files. e.g. --priv *.c,inc/*.h

       The 'extern' files are placed between 'priv1' and 'priv2'.

       The resulting code is packed as follows:

           /*
           [intro file contents]
           */

           #ifndef <macro>_SINGLE_HEADER
           #define <macro>_SINGLE_HEADER
           [public header file contents]
           #endif /* <macro>_SINGLE_HEADER */

           #ifdef <macro>_IMPLEMENTATION
           [private header and source file contents]
           #endif /* <macro>_IMPLEMENTATION */

           /*
           [outro file contents]
           */""")

def parse_files(arg):
    files = []
    paths = arg.split(",")
    for path in paths:
        if "*" in path:
            # Wildcard
            d = os.path.dirname(path)
            if d == "": d = "."
            if d == " ": continue
            if not os.path.exists(d):
                print(d + " does not exist.")
                exit()

            wildcard = os.path.basename(path)
            unsorted = []
            for file in os.listdir(d):
                if fnmatch.fnmatch(file, wildcard):
                    unsorted.append(os.path.join(d, file))
            unsorted.sort()
            files.extend(unsorted)

        else:
            # Regular file
            if not os.path.exists(path):
                print(path + " does not exist.")
                exit()
            elif os.path.isdir(path):
                print(path + " is a directory. Expected a file name.")
                exit()
            else:
                files.append(path)

    return files;

def omit_includes(str, files):
    for file in files:
        fname = os.path.basename(file)
        if ".h" in file:
            str = str.replace("#include \"" + fname + "\"", "");
            str = str.replace("#include <" + fname + ">", "");
    return str

def fix_comments(str):
    return re.sub(r"//(.*)(\n|$)", "/* \\1 */\\2", str)

# Main start
# ==========

if len(sys.argv) < 2:
    print_help()
    exit()

intro_files = []
pub_files = []
priv_files1 = []
outro_files2 = []
extern_files = []
cur_arg = 1
macro = ""

# Parse args
# ----------
while cur_arg < len(sys.argv):
    if sys.argv[cur_arg] == "--help":
        print_help()
        exit()
    elif sys.argv[cur_arg] == "--macro":
        cur_arg += 1
        macro = sys.argv[cur_arg]
    elif sys.argv[cur_arg] == "--intro":
        cur_arg += 1
        intro_files = parse_files(sys.argv[cur_arg])
    elif sys.argv[cur_arg] == "--pub":
        cur_arg += 1
        pub_files = parse_files(sys.argv[cur_arg])
    elif sys.argv[cur_arg] == "--priv1":
        cur_arg += 1
        priv_files1 = parse_files(sys.argv[cur_arg])
    elif sys.argv[cur_arg] == "--priv2":
        cur_arg += 1
        priv_files2 = parse_files(sys.argv[cur_arg])
    elif sys.argv[cur_arg] == "--extern":
        cur_arg += 1
        extern_files = parse_files(sys.argv[cur_arg])
    elif sys.argv[cur_arg] == "--outro":
        cur_arg += 1
        outro_files = parse_files(sys.argv[cur_arg])
    else:
        print("Unknown argument " + sys.argv[cur_arg])

    cur_arg += 1

if macro == "":
    print("Option --macro <macro> is mandatory")
    exit()

# Print concatenated output
# -------------------------
print("/*")
for f in intro_files:
    sys.stdout.write(open(f, 'r').read())
print("*/")

# print("\n#ifndef " + macro + "_SINGLE_HEADER");
# print("#define " + macro + "_SINGLE_HEADER");
print("#ifndef NK_SINGLE_FILE");
print("  #define NK_SINGLE_FILE");
print("#endif");
print("");

for f in pub_files:
    sys.stdout.write(open(f, 'r').read())
# print("#endif /* " + macro + "_SINGLE_HEADER */");

print("\n#ifdef " + macro + "_IMPLEMENTATION");
print("");

for f in priv_files1:
    print(omit_includes(open(f, 'r').read(),
                        pub_files + priv_files1 + priv_files2 + extern_files))
for f in extern_files:
    print(fix_comments(open(f, 'r').read()))

for f in priv_files2:
    print(omit_includes(open(f, 'r').read(),
                        pub_files + priv_files1 + priv_files2 + extern_files))

print("#endif /* " + macro + "_IMPLEMENTATION */");

print("\n/*")
for f in outro_files:
    sys.stdout.write(open(f, 'r').read())
print("*/\n")

