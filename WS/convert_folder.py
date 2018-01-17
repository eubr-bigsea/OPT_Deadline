import os
import re
import glob
import shutil
import sys

files_to_be_copied = []

def path_converter(parent_folder_name):
    def convert_path(match):
        path = match.group(1)
        span = match.span(1)
        file_content = ''
        with open(path, 'r') as f:
            file_content = f.read()

        files_to_be_copied.append((os.path.join(parent_folder_name, os.path.basename(path)), file_content))

        return match.string[match.span()[0]:span[0]] + "@@DIRPATH@@/" + parent_folder_name + "/" + os.path.basename(path) + match.string[span[1]:match.span()[1]]
    return convert_path

def convert_lua_file(lua_file_path):
    converted_file = []
    pattern = re.compile('solver\.fileToArray\("([^"]+)"\)')

    parent_folder_name = os.path.basename(lua_file_path)[:-4]

    with open(lua_file_path, 'r') as f:
        for line in f.readlines():
            converted_line = re.sub(pattern, path_converter(parent_folder_name), line)
            converted_file.append(converted_line)
    return '\n'.join(converted_file)

def make_dirs(path):
    os.makedirs(os.path.dirname(path), exist_ok=True)

def convert_folder(foldername, dest_app_files_folder):
    converted_lua_files = []

    luafiles_paths = glob.glob(os.path.join(foldername, '*.lua'))
    for luafile in luafiles_paths:
        files_to_be_copied.append((os.path.basename(luafile), convert_lua_file(luafile)))

    for f in files_to_be_copied:
        make_dirs(os.path.join(dest_app_files_folder, f[0]))
        with open(os.path.join(dest_app_files_folder, f[0]), 'w') as ff:
            ff.write(f[1])

    for txtfile in glob.glob(os.path.join(foldername, '*.txt')):
        shutil.copy2(txtfile, dest_app_files_folder)


    for csvfile in glob.glob(os.path.join(foldername, '*.csv')):
        shutil.copy2(csvfile, dest_app_files_folder)

if __name__ == '__main__':
    if len(sys.argv) < 3:
        print("Usage: python {} <folder> <output_folder>".format(sys.argv[0]))
        sys.exit(0)
    else:
        convert_folder(sys.argv[1], sys.argv[2])
