CMAKE_MINIMUM_REQUIRED(VERSION 3.0)

SET(USE_MKL ON)
SET(MPI ON)

FIND_PROGRAM(CMAKE_C_COMPILER NAMES $ENV{CC} gcc PATHS ENV PATH NO_DEFAULT_PATH)
FIND_PROGRAM(CMAKE_CXX_COMPILER NAMES $ENV{CXX} g++ PATHS ENV PATH NO_DEFAULT_PATH)

IF (${BUILD_OH})
    SET(PROJECT_NAME block.oh)
ELSEIF (${BUILD_READROT})
    SET(PROJECT_NAME block.read_rot)
ELSEIF (${BUILD_GAOPT})
    SET(PROJECT_NAME block.gaopt)
ELSE()
    SET(PROJECT_NAME block.spin_adapted)
ENDIF()

PROJECT(${PROJECT_NAME} VERSION 1.5.3)

MESSAGE(STATUS "PROJECT_NAME = ${PROJECT_NAME}")

SET(CMAKE_CXX_STANDARD 11)
SET(CMAKE_CXX_STANDARD_REQUIRED True)

SET(Python_ADDITIONAL_VERSIONS 3)
FIND_PACKAGE(PythonInterp)

IF (NOT PYTHONINTERP_FOUND)
    MESSAGE(FATAL_ERROR "Python3 not found.")
ENDIF()

FIND_PACKAGE(PythonLibs ${PYTHON_VERSION_MAJOR}.${PYTHON_VERSION_MINOR})

EXECUTE_PROCESS(COMMAND ${PYTHON_EXECUTABLE} -c "import distutils.sysconfig; \
    print(distutils.sysconfig.get_config_var('LIBDIR'))"
    OUTPUT_VARIABLE PYTHON_LIB_PATH OUTPUT_STRIP_TRAILING_WHITESPACE)
EXECUTE_PROCESS(COMMAND ${PYTHON_EXECUTABLE} -c "import distutils.sysconfig; \
    print(distutils.sysconfig.get_config_var('INCLUDEDIR'))"
    OUTPUT_VARIABLE PYTHON_INCLUDE_PATH OUTPUT_STRIP_TRAILING_WHITESPACE)

IF (NOT PYTHONLIBS_FOUND)
    EXECUTE_PROCESS(COMMAND ${PYTHON_EXECUTABLE} -c "import distutils.sysconfig; \
        import os; print(os.path.join(distutils.sysconfig.get_config_var('LIBDIR'), \
        distutils.sysconfig.get_config_var('LDLIBRARY')))"
        OUTPUT_VARIABLE PYTHON_LIBRARIES OUTPUT_STRIP_TRAILING_WHITESPACE)
    EXECUTE_PROCESS(COMMAND ${PYTHON_EXECUTABLE} -c "from distutils.sysconfig import get_python_inc; \
        print(get_python_inc())"
        OUTPUT_VARIABLE PYTHON_INCLUDE_DIRS OUTPUT_STRIP_TRAILING_WHITESPACE)
ENDIF()

EXECUTE_PROCESS(COMMAND ${PYTHON_EXECUTABLE} -c "import importlib.machinery; \
    print(importlib.machinery.EXTENSION_SUFFIXES[0])"
    OUTPUT_VARIABLE PYLIB_SUFFIX OUTPUT_STRIP_TRAILING_WHITESPACE)

MESSAGE(STATUS "PROJECT_NAME = ${PROJECT_NAME}")
MESSAGE(STATUS "PYTHON_VERSION_MAJOR = ${PYTHON_VERSION_MAJOR}")
MESSAGE(STATUS "PYTHON_VERSION_MINOR = ${PYTHON_VERSION_MINOR}")
MESSAGE(STATUS "PYTHON_LIBRARIES = ${PYTHON_LIBRARIES}")
MESSAGE(STATUS "PYTHON_EXECUTABLE = ${PYTHON_EXECUTABLE}")
MESSAGE(STATUS "PYTHON_EXECUTABLE_HINT = ${PYTHON_EXECUTABLE_HINT}")
MESSAGE(STATUS "PYTHON_INCLUDE_DIRS = ${PYTHON_INCLUDE_DIRS}")
MESSAGE(STATUS "PYLIB_SUFFIX = ${PYLIB_SUFFIX}")

IF (${PYTHON_EXECUTABLE_HINT})
    IF (NOT (${PYTHON_EXECUTABLE_HINT} EQUAL ${PYTHON_EXECUTABLE}))
        MESSAGE(FATAL_ERROR "Python3 used by cmake (${PYTHON_EXECUTABLE}) does not match Python3 \
            used by setup.py (${PYTHON_EXECUTABLE_HINT})!")
    ENDIF()
ENDIF()

IF (NOT(APPLE) AND NOT(WIN32))
    SET(NO_AS_NEEDED -Wl,--no-as-needed)
ELSEIF (CMAKE_CXX_COMPILER_ID STREQUAL "AppleClang")
    SET(XPREP -Xpreprocessor)
    INCLUDE_DIRECTORIES(/usr/local/include)
ENDIF()

SET(PTHREAD pthread)

ADD_SUBDIRECTORY(newmat10)

IF (${BOOST_OLD})
    SET(BOOST_FLAG "-DBOOST_NO_CXX11_SCOPED_ENUMS")
ELSE()
    SET(BOOST_FLAG -DBOOST_1_56_0 -DBOOST_BIND_GLOBAL_PLACEHOLDERS)
ENDIF()

IF (CMAKE_CXX_COMPILER_ID STREQUAL "Intel")
    SET(OMP_FLAG -qopenmp)
ELSEIF (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    SET(OMP_FLAG -openmp)
ELSE()
    SET(OMP_FLAG -fopenmp)
ENDIF()

IF (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    SET(OPT_FLAG ${OMP_FLAG} -bigobj -MP)
ELSEIF("${CMAKE_BUILD_TYPE}" STREQUAL "Debug")
    SET(OPT_FLAG -O0 -g ${XPREP} ${OMP_FLAG} -fstack-check -fno-omit-frame-pointer -fno-optimize-sibling-calls
        -Werror -Werror=return-type)
ELSE()
    SET(OPT_FLAG -O2 -funroll-loops ${XPREP} ${OMP_FLAG} -Werror -Wno-deprecated-declarations)
ENDIF()

IF ((NOT APPLE) AND (NOT WIN32) AND (NOT (CMAKE_CXX_COMPILER_ID STREQUAL "Intel")))
    SET(OPT_FLAG ${OPT_FLAG} -Wno-psabi)
ENDIF()

IF (${USE_MKL})
    SET(CMAKE_FIND_LIBRARY_SUFFIXES_BKP ${CMAKE_FIND_LIBRARY_SUFFIXES})
    SET(CMAKE_FIND_LIBRARY_SUFFIXES "${CMAKE_FIND_LIBRARY_SUFFIXES_BKP};.so.1;.1.dylib;.so.2;.2.dylib;.so.3;.3.dylib;.so.4;.4.dylib")
    FIND_PATH(MKL_INCLUDE_DIR NAMES mkl.h HINTS $ENV{MKLROOT}/include /usr/local/include ${PYTHON_INCLUDE_PATH})
    FIND_LIBRARY(MKL_LIB_LP NAMES mkl_intel_lp64
        PATHS $ENV{MKLROOT}/lib $ENV{MKLROOT}/lib/intel64 /usr/local/lib ${PYTHON_LIB_PATH} NO_DEFAULT_PATH)
    SET(MKL_FLAG "-D_HAS_INTEL_MKL")
    FIND_LIBRARY(MKL_LIB_CORE NAMES mkl_core
        PATHS $ENV{MKLROOT}/lib $ENV{MKLROOT}/lib/intel64 /usr/local/lib ${PYTHON_LIB_PATH} NO_DEFAULT_PATH)
    FIND_LIBRARY(MKL_LIB_GT NAMES mkl_sequential
        PATHS $ENV{MKLROOT}/lib $ENV{MKLROOT}/lib/intel64 /usr/local/lib ${PYTHON_LIB_PATH} NO_DEFAULT_PATH)
    FIND_LIBRARY(MKL_LIB_AVX NAMES mkl_avx2
        PATHS $ENV{MKLROOT}/lib $ENV{MKLROOT}/lib/intel64 /usr/local/lib ${PYTHON_LIB_PATH} NO_DEFAULT_PATH)
    FIND_LIBRARY(MKL_LIB_AVX512 NAMES mkl_avx512
        PATHS $ENV{MKLROOT}/lib $ENV{MKLROOT}/lib/intel64 /usr/local/lib ${PYTHON_LIB_PATH} NO_DEFAULT_PATH)
    IF (NOT MKL_LIB_AVX)
        SET(MKL_LIB_AVX "")
    ENDIF()
    IF (NOT MKL_LIB_AVX512)
        SET(MKL_LIB_AVX512 "")
    ENDIF()
    SET(MKL_LIBS ${NO_AS_NEEDED} ${PTHREADS} ${MKL_LIB_LP} ${MKL_LIB_CORE} ${MKL_LIB_GT} ${MKL_LIB_AVX} ${MKL_LIB_AVX512})
    SET(CMAKE_FIND_LIBRARY_SUFFIXES ${CMAKE_FIND_LIBRARY_SUFFIXES_BKP})
    MESSAGE(STATUS "MKL_INCLUDE_DIR = ${MKL_INCLUDE_DIR}")
    MESSAGE(STATUS "MKL_LIBS = ${MKL_LIBS}")
ELSE()
    # Check LAPACK and BLAS
    FIND_PACKAGE(BLAS REQUIRED)
    FIND_PACKAGE(LAPACK REQUIRED)

    SET(MKL_INCLUDE_DIR "")
    SET(MKL_LIBS "")
    SET(MKL_FLAG "")
ENDIF()

SET(OMP_LIB_BASE_NAME gomp)

IF (CMAKE_CXX_COMPILER_ID STREQUAL "Intel")
    SET(OMP_LIB_BASE_NAME iomp5)
ELSEIF (CMAKE_CXX_COMPILER_ID STREQUAL "AppleClang")
    SET(OMP_LIB_BASE_NAME iomp5)
ENDIF()

FIND_LIBRARY(OMP_LIB_NAME NAMES ${OMP_LIB_BASE_NAME} PATHS /usr/local/lib /usr/lib64 ${PYTHON_LIB_PATH})
IF (NOT OMP_LIB_NAME)
    EXECUTE_PROCESS(
        COMMAND ${CMAKE_CXX_COMPILER} -print-search-dirs
        COMMAND grep "^lib" COMMAND awk -F "=" "{print $2}" COMMAND tr ":" ";"
        OUTPUT_VARIABLE OMP_LIB_HINT OUTPUT_STRIP_TRAILING_WHITESPACE)
    FIND_LIBRARY(OMP_LIB_NAME NAMES ${OMP_LIB_BASE_NAME} PATHS ${OMP_LIB_HINT})
ENDIF()

SET(SRC_genetic genetic/CrossOver.C genetic/Evaluate.C genetic/GAInput.C genetic/GAOptimize.C
    genetic/Generation.C genetic/Mutation.C genetic/RandomGenerator.C genetic/ReadIntegral.C)

SET(SRC_npdm modules/npdm/npdm.C modules/npdm/npdm_driver.C modules/npdm/npdm_patterns.C
    modules/npdm/npdm_expectations.C modules/npdm/npdm_expectations_engine.C
    modules/npdm/npdm_permutations.C modules/npdm/npdm_spin_adaptation.C modules/npdm/npdm_operator_selector.C
    modules/npdm/npdm_spin_ops.C modules/npdm/onepdm_container.C modules/npdm/twopdm_container.C
    modules/npdm/threepdm_container.C modules/npdm/fourpdm_container.C modules/two_index_ops/two_index_wrappers.C
    modules/three_index_ops/three_index_wrappers.C modules/four_index_ops/four_index_wrappers.C
    modules/three_index_ops/three_index_compound_ops.C modules/four_index_ops/four_index_compound_ops.C
    modules/three_index_ops/three_index_op_components.C modules/four_index_ops/four_index_op_components.C
    modules/three_index_ops/three_index_ops.C modules/four_index_ops/four_index_ops.C
    modules/three_index_ops/build_3index_ops.C modules/four_index_ops/build_4index_ops.C modules/npdm/pairpdm_container.C
    modules/npdm/nevpt2_npdm_driver.C modules/npdm/nevpt2_container.C modules/npdm/externalsort.C
    modules/three_index_ops/build_3index_singleoperator.C modules/npdm/npdm_driver_2.C)

SET(SRC_mps_nevpt modules/mps_nevpt/nevpt_op_component.C modules/mps_nevpt/type1.C modules/mps_nevpt/mps_nevpt.C)

SET(SRCS saveBlock.C Stackspinblock.C operatorUtilities.C Stack_op_components.C stackguess_wavefunction.C
    Stackwavefunction.C Stackdensity.C stackopxop.C StackBaseOperator.C StackOperators.C
    modules/ResponseTheory/sweepResponse.C modules/ResponseTheory/sweepCompress.C pario.C
    dmrg.C fiedler.C least_squares.C sweep_mps.C set_stackspinblock_components.C linear.C main.C
    readinput.C timer.C SpinQuantum.C Symmetry.C input.C hfOccGenerator.C Schedule.C orbstring.C
    slater.C csf.C StateInfo.C  screen.C MatrixBLAS.C operatorfunctions.C solver.C davidson.C
    sweep_params.C sweep.C partial_sweep.C BlockAndDecimate.C initblocks.C rotationmat.C
    renormalise.C couplingCoeffs.C distribute.C new_anglib.C fci.C IrrepSpace.C modules/generate_blocks/sweep.C
    ${SRC_genetic} SpinSpace.C include/IntegralMatrix.C modules/onepdm/sweep.C modules/onepdm/onepdm.C
    ${SRC_npdm} ${SRC_mps_nevpt} fciqmchelper.C)

IF (${BUILD_OH})
    SET(SRCS wrapper.C fciqmchelper.C saveBlock.C Stackspinblock.C operatorUtilities.C Stack_op_components.C
        stackguess_wavefunction.C Stackwavefunction.C Stackdensity.C stackopxop.C StackBaseOperator.C StackOperators.C
        modules/ResponseTheory/sweepResponse.C modules/ResponseTheory/sweepCompress.C pario.C dmrg.C fiedler.C
        least_squares.C sweep_mps.C set_stackspinblock_components.C linear.C readinput.C  timer.C SpinQuantum.C Symmetry.C
        input.C hfOccGenerator.C Schedule.C orbstring.C slater.C csf.C StateInfo.C  screen.C MatrixBLAS.C
        operatorfunctions.C solver.C davidson.C sweep_params.C sweep.C partial_sweep.C BlockAndDecimate.C initblocks.C
        rotationmat.C renormalise.C couplingCoeffs.C distribute.C new_anglib.C fci.C IrrepSpace.C
        modules/generate_blocks/sweep.C ${SRC_genetic} SpinSpace.C include/IntegralMatrix.C modules/onepdm/sweep.C
        modules/onepdm/onepdm.C ${SRC_npdm} OverlapHelement.C ${SRC_mps_nevpt})
ELSEIF (${BUILD_READROT})
    SET(SRCS wrapper.C fciqmchelper.C saveBlock.C Stackspinblock.C operatorUtilities.C Stack_op_components.C
        stackguess_wavefunction.C Stackwavefunction.C Stackdensity.C stackopxop.C StackBaseOperator.C StackOperators.C
        modules/ResponseTheory/sweepResponse.C modules/ResponseTheory/sweepCompress.C pario.C dmrg.C fiedler.C
        least_squares.C sweep_mps.C set_stackspinblock_components.C linear.C readinput.C  timer.C SpinQuantum.C Symmetry.C
        input.C hfOccGenerator.C Schedule.C orbstring.C slater.C csf.C StateInfo.C  screen.C MatrixBLAS.C
        operatorfunctions.C solver.C davidson.C sweep_params.C sweep.C partial_sweep.C BlockAndDecimate.C initblocks.C
        rotationmat.C renormalise.C couplingCoeffs.C distribute.C new_anglib.C fci.C IrrepSpace.C
        modules/generate_blocks/sweep.C ${SRC_genetic} SpinSpace.C include/IntegralMatrix.C modules/onepdm/sweep.C
        modules/onepdm/onepdm.C ${SRC_npdm} readrotationmatrix.C ${SRC_mps_nevpt})
ELSEIF (${BUILD_GAOPT})
    SET(SRCS genetic/main.C genetic/fiedler.C ${SRC_genetic})
ENDIF()

SET(INCL . newmat10 include modules/generate_blocks modules/onepdm modules/twopdm modules/npdm
    modules/two_index_ops modules/three_index_ops modules/four_index_ops
    modules/ResponseTheory modules/nevpt2 molcas modules/mps_nevpt)

ADD_EXECUTABLE(${PROJECT_NAME} ${SRCS})
TARGET_INCLUDE_DIRECTORIES(${PROJECT_NAME} PUBLIC ${INCL})

SET(Boost_USE_MULTITHREADED ON)
FIND_PACKAGE(Boost REQUIRED COMPONENTS system filesystem serialization)

TARGET_INCLUDE_DIRECTORIES(${PROJECT_NAME} PUBLIC ${Boost_INCLUDE_DIRS} ${MKL_INCLUDE_DIR} ${PYTHON_INCLUDE_DIRS})
TARGET_LINK_LIBRARIES(${PROJECT_NAME} PUBLIC newmat ${PTHREAD} ${OMP_LIB_NAME})
TARGET_LINK_LIBRARIES(${PROJECT_NAME} PUBLIC ${LAPACK_LIBRARIES} ${BLAS_LIBRARIES} ${MKL_LIBS} ${Boost_LIBRARIES})
TARGET_COMPILE_OPTIONS(${PROJECT_NAME} PUBLIC ${BOOST_FLAG} -DBLAS -DUSELAPACK -D_HAS_CBLAS -DFAST_MTP ${OPT_FLAG} ${MKL_FLAG})

IF (${MPI})
    FIND_PACKAGE(MPI REQUIRED)
    FIND_PACKAGE(Boost REQUIRED COMPONENTS mpi)
    SET_TARGET_PROPERTIES(${PROJECT_NAME} PROPERTIES COMPILE_FLAGS "${MPI_COMPILE_FLAGS}")
    SET_TARGET_PROPERTIES(${PROJECT_NAME} PROPERTIES LINK_FLAGS "${MPI_LINK_FLAGS}")
    TARGET_INCLUDE_DIRECTORIES(${PROJECT_NAME} PUBLIC ${MPI_INCLUDE_PATH})
    TARGET_LINK_LIBRARIES(${PROJECT_NAME} PUBLIC ${MPI_CXX_LIBRARIES} ${Boost_LIBRARIES})
    MESSAGE(STATUS "MPI_COMPILE_FLAGS = ${MPI_COMPILE_FLAGS}")
    MESSAGE(STATUS "MPI_LINK_FLAGS = ${MPI_LINK_FLAGS}")
    MESSAGE(STATUS "MPI_INCLUDE_PATH = ${MPI_INCLUDE_PATH}")
    MESSAGE(STATUS "MPI_CXX_LIBRARIES = ${MPI_CXX_LIBRARIES}")
ELSE()
    TARGET_COMPILE_OPTIONS(${PROJECT_NAME} PUBLIC -DSERIAL)
ENDIF()

MESSAGE(STATUS "SRCS = ${SRCS}")
MESSAGE(STATUS "Boost_VERSION = ${Boost_VERSION}")
MESSAGE(STATUS "Boost_INCLUDE_DIRS = ${Boost_INCLUDE_DIRS}")
