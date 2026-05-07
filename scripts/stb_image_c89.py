"""
Generates a C89-compatible version of stb_image.h
by replacing C++ comments with equivalent C-style comments.
"""

def main() -> None:
    inside_c_comment = False
    cpp_comments = []
    with open("../demo/common/stb_image.h") as f:
        for line in f:
            # Avoid replacing anything in existing C-style comments
            if inside_c_comment:
                print(line, end="")
                if "*/" in line:
                    inside_c_comment = False
                continue
            if "/*" in line:
                print(line, end="")
                inside_c_comment = "*/" not in line
                continue

            # Replace C++ comments with C-style comments
            lx = line.find("//")
            if lx != -1 and not line[:lx].strip():
                # This could be part of a multi-line comment
                cpp_comments.append(line)
                continue

            if cpp_comments:
                comment = cpp_comments[0]
                cx = comment.find("//")
                if len(cpp_comments) == 1:
                    print(f"{comment[:cx]}/*{comment[cx+2:].rstrip('\n')} */\n", end="")
                else:
                    print(f"{comment[:cx]}/*{comment[cx+2:]}", end="")

                    for comment in cpp_comments[1:-1]:
                        cx = comment.find("//")
                        print(f"{comment[:cx]} *{comment[cx+2:]}", end="")

                    comment = cpp_comments[-1]
                    cx = comment.find("//")
                    if comment[cx+2:].rstrip('\n'):
                        print(f"{comment[:cx]} *{comment[cx+2:].rstrip('\n')} */\n", end="")
                    else:
                        print(f"{comment[:cx]} */\n", end="")

                cpp_comments.clear()

            if lx != -1:
                # Inline comment
                print(f"{line[:lx]}/*{line[lx+2:].rstrip('\n')} */\n", end="")
            else:
                print(line, end="")


if __name__ == "__main__":
    main()
