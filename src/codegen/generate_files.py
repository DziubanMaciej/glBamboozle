import sys
import os
import csv
from pathlib import Path

# Parse arguments
if len(sys.argv) < 5:
    print("Usage:")
    print("  generate_files.py INPUT_CSV OUTPUT_DIR ENABLE_LOGS GLBAMBOOZLE_ENABLE_LOGS_END")
    sys.exit(1)
input_csv_path = sys.argv[1]
output_dir_path = Path(sys.argv[2])
enable_logs = (sys.argv[3] == '1')
log_end = (sys.argv[4] == '1')
output_dir_path.mkdir(exist_ok=True)
bamboozled_functions = [
    "wglSwapBuffers",
    "wglSwapLayerBuffers",
]

def get_typedef_name(name):
    return f"PFN_{name}"

def construct_argslist(args):
    args_list = [f"{x[0]} {x[1]}" for x in args]
    args_list = ', '.join(args_list)
    return args_list

def parse_args(args):
    result = []
    for i in range(len(args)//2):
        name = args[2*i]
        value = args[2*i+1]
        result.append((name, value))
    return result

def parse_row(row):
    return_type = row[0]
    name = row[1]
    args = row[2:]
    args = parse_args(args)
    return return_type, name, args

with open(input_csv_path, 'r') as csv_file:
    gl_functions = list(csv.reader(csv_file))
    gl_functions = [parse_row(x) for x in gl_functions]


with open(output_dir_path / "real_entrypoints.h", 'w') as out_file:
    out_file.write("#pragma once\n")
    out_file.write("\n")
    out_file.write('#include <Windows.h>\n')
    out_file.write('#include <gl/GL.h>\n')
    out_file.write("\n")

    for return_type, name, args in gl_functions:
        args_list = construct_argslist(args)
        line = f"using {get_typedef_name(name)} = {return_type}(WINAPI *)({args_list});\n"
        out_file.write(line)

    out_file.write("\nstruct RealEntryPoints {\n")
    out_file.write("    void load(HMODULE realLibrary);\n")
    out_file.write("    PROC getProc(LPCSTR procName);\n")
    out_file.write("\n")
    for return_type, name, args in gl_functions:
        line = f"    {get_typedef_name(name)} {name};\n"
        out_file.write(line)
    out_file.write("};\n")

with open(output_dir_path / "real_entrypoints.cpp", 'w') as out_file:
    out_file.write('#include "global_state.h"\n')
    out_file.write('#include "log.h"\n')
    out_file.write("\n")

    out_file.write('extern "C" {\n')
    for return_type, name, args in gl_functions:
        proxy_name = f"{name}Proxy"
        args_list = construct_argslist(args)
        args_values = ', '.join([x[1] for x in args])
        calling_convention = "APIENTRY"
        has_result = return_type != "void"
        result_assignment = f"{return_type} result = " if has_result else ""
        return_value = " result" if return_type != "void" else ""
        global_state_getter = "getBamboozler" if name in bamboozled_functions else "getRealEntrypoints"

        out_file.write(f"    {return_type} {calling_convention} {proxy_name}({args_list})\n")
        out_file.write(f"    {{\n")
        out_file.write(f'        #pragma comment(linker, "/export:{name}=" __FUNCDNAME__)\n')
        if enable_logs:
            out_file.write(f'        DEBUG_LOG("glBamboozle: {name}");\n')
        if name in bamboozled_functions:
            out_file.write(f"        GlobalState::instance().getExtensions().load(GlobalState::instance().getRealEntrypoints());\n")
        out_file.write(f"        {result_assignment}GlobalState::instance().{global_state_getter}().{name}({args_values});\n")
        if enable_logs and log_end:
                out_file.write(f'        DEBUG_LOG("glBamboozle: {name} END\\n");\n')
        if has_result:
            out_file.write(f"        return result;\n")
        out_file.write(f"    }}\n")
        out_file.write(f"\n")
    out_file.write("}\n")

    out_file.write("void RealEntryPoints::load(HMODULE realLibrary) {\n")
    for return_type, name, args in gl_functions:
        typedef_name = get_typedef_name(name)
        out_file.write(f'    {name} = reinterpret_cast<{typedef_name}>(GetProcAddress(realLibrary, "{name}"));\n')
    out_file.write("}\n\n")

    out_file.write("PROC RealEntryPoints::getProc(LPCSTR procName) {\n")
    for return_type, name, args in gl_functions:
        proxy_name = f"{name}Proxy"

        out_file.write(f'    if (strcmp(procName, "{name}")) {{\n')
        out_file.write(f"        return reinterpret_cast<PROC>({proxy_name});\n")
        out_file.write(f"    }}\n")
    out_file.write("    return NULL;\n")
    out_file.write("}\n\n")
