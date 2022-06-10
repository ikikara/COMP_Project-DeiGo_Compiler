# COMP_Project-DeiGo_Compiler
- [ ] Finished
- [ ] Solving memory leaks
- [ ] Finish Code Generation

## Index
- [Description](#description)
- [Technologies used](#technologies-used)
- [To run this project](#to-run-this-project)
- [Notes important to read](#notes-important-to-read)
- [Authors](#authors)

## Description
This project was developed for Compilers subject @University of Coimbra, Informatics Engineering <br>
Consists in develop a program that works as a Compiler for a branch of the language Golang, the language DeiGo. 

#### Main Languages:
![](https://img.shields.io/badge/-C-333333?style=flat&logo=C%2B%2B&logoColor=5459E2) 

## Technologies used:
1. C ([documentation](https://devdocs.io/c/))
2. Lex
3. Yacc
4. LLVM
5. Clang

## To run this project:
You have multiple ways to run this project:
  * Download the folder "src" and compile the files using the bash compile.sh:
    ```shellscript
    [your-disk]:[name-path]> sh compile.sh gocompiler
    ```
  * After compiling you will run the project with the following flags:
    + Lexical Analysis (-l):
      ```shellscript
      [your-disk]:[name-path]> ./gocompiler -l < file.txt
      ```
    + Syntatic Analysis (-t):
      ```shellscript
      [your-disk]:[name-path]> ./gocompiler -t < file.txt
      ```
    + Semantic Analysis (-s):
      ```shellscript
      [your-disk]:[name-path]> ./gocompiler -s < file.txt
      ```
    + Code generation (no flag) using the bash compile_llvm.sh
      ```shellscript
      [your-disk]:[name-path]> sh compile_llvm.sh [name-of-file.txt-without-.txt]
      ```
      This will generates a file .llvm that will have the assembly code of the original code. Then generates the executable of the program, to run it:
      ```shellscript
      [your-disk]:[name-path]> ./[name-of-file.txt-without-.txt]
      ```


## Notes important to read:
- This project was developed in Linux, so it's recommended to use it on Linux or a terminal WSL (Visual Studio).
- As said before this project was developed in Linux so you will need to have Lex/Yacc/LLVM/Clang in installed "there".
- The code generation wasn't finished yet, so some functionalities will give some errors in LLVM or not work as expected.
- For more information about how to use this project and how this DeiGO language works consult the Statement and Reports.
- The folder "tests" have tests used while developing this project to verify that our program is returning the correct output.
  * goal1 -> Lexical Analysis
  * goal2 -> Syntatic Analysis
  * goal3 -> Semantic Analysic

## Authors:
- [João Silva](https://github.com/ikikara)
- [Pedro Martins](https://github.com/PedroMartinsUC)
