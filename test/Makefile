CXX = g++
CFLAGS = -fno-rtti -std=c++11

LLVM_SRC_PATH = /ssd/CodeProject/llvm
LLVM_BUILD_PATH = /ssd/CodeProject/build

LLVM_BIN_PATH = $(LLVM_BUILD_PATH)/bin
LLVM_LIBS=core mc

LLVM_CXXFLAGS := `$(LLVM_BIN_PATH)/llvm-config --cxxflags`
LLVM_LDFLAGS := `$(LLVM_BIN_PATH)/llvm-config --ldflags --libs --system-libs`


CLANG_BUILD_FLAGS = -I$(LLVM_SRC_PATH)/tools/clang/include \
                                      -I$(LLVM_BUILD_PATH)/tools/clang/include

CLANGLIBS = \
  -lclangFrontendTool -lclangFrontend -lclangDriver \
  -lclangSerialization -lclangCodeGen -lclangParse \
  -lclangSema -lclangStaticAnalyzerFrontend \
  -lclangStaticAnalyzerCheckers -lclangStaticAnalyzerCore \
  -lclangAnalysis -lclangARCMigrate -lclangRewrite \
  -lclangEdit -lclangAST -lclangLex -lclangBasic

all: rewritersample

rewritersample: rewritersample.cpp
	$(CXX) rewritersample.cpp $(CFLAGS) $(LLVM_CXXFLAGS) -o rewritersample $(CLANG_BUILD_FLAGS) $(CLANGLIBS) $(LLVM_LDFLAGS)





