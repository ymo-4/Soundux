from pathlib import Path

def replace_until_no_changes(line: str, old: str, new: str) -> str:
    while True:
        l = len(line)
        line = line.replace(old, new)
        if l == len(line):
            return line


p = Path("input-event-codes.h")
f = p.open('r')
lines = f.readlines()
f.close()

global output_is, output_si
includes = """
#pragma once
#include <string>
#include <unordered_map>

"""

output_si = includes + "static const std::unordered_map<std::string, int> event_codes_si = {\n" # } identation workaround
output_is = includes + "static const std::unordered_map<int, std::string> event_codes_is = {\n" # }


for line in lines:
    if line.startswith("#define"):
        # replace all tabs with spaces
        line = replace_until_no_changes(line, "\t", " ")
        # will remove all double-spaces in the line
        line = replace_until_no_changes(line, "  ", " ")
        splited = line.split(' ')
        
        try:
            s = splited[1]
            i = splited[2]
            if i.startswith("0x"):
                i = int(i, 16)
            else:
                i = int(i)

            output_si += '{"' + s + '", ' + str(i) + '},\n'
            output_is += '{' + str(i) + ', "' + s + '"},\n'

        except:
            pass

output_si += "};"
output_is += "};"

f = Path("event-codes-si.h").open('w')
f.write(output_si)
f.close()
 
f = Path("event-codes-is.h").open('w')
f.write(output_is)
f.close()
