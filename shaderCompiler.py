import pathlib
import os
import time
from sys import platform, exit
if platform != "win32":
    import pwd

COMPDIR_FILE = "compDir.save"

def getUsername():
    return pwd.getpwuid(os.getuid())[0]

def findConsistingInFolder(folder, searchKey):
    for root, _, files in os.walk(folder):
        for f in files:
            if f.find(searchKey) == 0:
                return root + "/" + f

def getCompilerExe():
    try:
        f = open(COMPDIR_FILE, "r")
        compDir = f.read()
        f.close()
        print(f"Found compiler dir : {compDir}\n")
        return compDir

    except FileNotFoundError:
        print("Could not find compiler specifier file will search for it now\n")
        if platform == "darwin":
            searchPath = f"/Users/{getUsername()}/VulkanSDK"
            compDir = findConsistingInFolder(searchPath, "glslc")
        elif platform == "linux" or platform == "linux2":
            compDir = "/usr/bin/glslc"
        elif platform == "win32":
            searchPath = "C:/VulkanSDK"
            compDir = findConsistingInFolder(searchPath, "glslc")
        else:
            exit("Could not recognize the platform")

        print(f"Found compiler dir : {compDir}\n")
        f = open(COMPDIR_FILE, "w")
        f.write(compDir)
        f.close()

        return compDir;

compilerExe = getCompilerExe()

pathOfFile = os.path.dirname(os.path.abspath(__file__))

shaderFolders = ["assets/shaders"] # folders to check for shader files

fileMap = {}

def getChangedFiles():
    global fileMap, shaderFolders

    changedFiles = []
    for shaderFolder in shaderFolders:
        l = os.walk(pathOfFile + "/" + shaderFolder)
        
        for files in l:
            for file in files[2]:
                if file.split(".")[-1] == "spv":
                    continue
                filePath = files[0] + "/" + file
                fileN = pathlib.Path(filePath)
                if filePath not in fileMap:
                    changedFiles.append(filePath)
                else:
                    if (fileN.stat().st_mtime != fileMap[filePath]):
                        changedFiles.append(filePath)
                fileMap[filePath] = fileN.stat().st_mtime

    return changedFiles

def getFolder(fullLocation):
    return "/".join(fullLocation.split("/")[:-1])

def getName(fullLocation):
    return "".join(fullLocation.split("/")[-1].split(".")[:-1])

def compileShaders(shaderFiles):
    for shaderFile in shaderFiles:
        # -O optimizes the shader for better performance
        commandString = compilerExe + " -O " + shaderFile + " -o " + shaderFile + ".spv"
        print(commandString)
        os.system(commandString)

while 1:
    changedFiles = getChangedFiles()
    if (len(changedFiles) > 0):
        print(len(changedFiles), "Shader file changed. Compling now...\n")
        compileShaders(changedFiles)
        print("\nCompilation done\n")

    time.sleep(0.1)