CMAKE_MINIMUM_REQUIRED(VERSION 3.0)

SET(USE_MKL ON)
SET(OMP_LIB GNU)
SET(MPI ON)

FIND_PROGRAM(CMAKE_C_COMPILER NAMES $ENV{CC} gcc PATHS ENV PATH NO_DEFAULT_PATH)
FIND_PROGRAM(CMAKE_CXX_COMPILER NAMES $ENV{CXX} g++ PATHS ENV PATH NO_DEFAULT_PATH)

IF (${BUILD_OH})
    SET(PROJECT_NAME OH)
ELSEIF (${BUILD_READROT})
    SET(PROJECT_NAME read_rot)
ELSEIF (${BUILD_GAOPT})
    SET(PROJECT_NAME gaopt)
ELSE()
    SET(PROJECT_NAME block.spin_adapted)
ENDIF()

PROJECT(${PROJECT_NAME} VERSION 1.5.3)

MESSAGE(STATUS "PROJECT_NAME = ${PROJECT_NAME}")

SET(CMAKE_CXX_STANDARD 11)
SET(CMAKE_CXX_STANDARD_REQUIRED True)

ADD_SUBDIRECTORY(newmat10)

IF (${BOOST_OLD})
    SET(BOOST_FLAG "-DBOOST_NO_CXX11_SCOPED_ENUMS")
ELSE()
    SET(BOOST_FLAG -DBOOST_1_56_0 -DBOOST_BIND_GLOBAL_PLACEHOLDERS)
ENDIF()

IF("${OMP_LIB}" STREQUAL "SEQ")
    SET(OMPFLAG "")
ELSEIF ("${OMP_LIB}" STREQUAL "GNU")
    SET(OMPFLAG "-fopenmp")
ELSE()
    SET(OMPFLAG "-fopenmp")
ENDIF()

IF (${USE_MKL})
    FIND_PATH(MKL_INCLUDE_DIR NAMES mkl.h HINTS $ENV{MKLROOT}/include)
    IF("${OMP_LIB}" STREQUAL "SEQ")
        SET(MKL_LIBS -L$ENV{MKLROOT}/lib/intel64 -Wl,--no-as-needed -lmkl_intel_lp64 -lmkl_sequential -lmkl_core -lpthread -lm -ldl)
    ELSEIF ("${OMP_LIB}" STREQUAL "GNU")
        SET(MKL_LIBS -L$ENV{MKLROOT}/lib/intel64 -Wl,--no-as-needed -lmkl_intel_lp64 -lmkl_gnu_thread -lmkl_core -lgomp -lpthread -lm -ldl)
    ELSE()
        SET(MKL_LIBS -L$ENV{MKLROOT}/lib/intel64 -Wl,--no-as-needed -lmkl_intel_lp64 -lmkl_intel_thread -lmkl_core -liomp5 -lpthread -lm -ldl)
    ENDIF()
    MESSAGE(STATUS "MKL_INCLUDE_DIR = ${MKL_INCLUDE_DIR}")
    MESSAGE(STATUS "MKL_LIBS = ${MKL_LIBS}")
    SET(MKL_FLAG "-D_HAS_INTEL_MKL")
ELSE()
    # Check LAPACK and BLAS
    FIND_PACKAGE(BLAS REQUIRED)
    FIND_PACKAGE(LAPACK REQUIRED)
    
    SET(MKL_INCLUDE_DIR "")
    SET(MKL_LIBS "")
    SET(MKL_FLAG "")
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
TARGET_INCLUDE_DIRECTORIES(${PROJECT_NAME} PUBLIC ${Boost_INCLUDE_DIRS} ${MKL_INCLUDE_DIR})
TARGET_LINK_LIBRARIES(${PROJECT_NAME} PUBLIC newmat pthread rt)
TARGET_LINK_LIBRARIES(${PROJECT_NAME} PUBLIC ${LAPACK_LIBRARIES} ${BLAS_LIBRARIES} ${MKL_LIBS} ${Boost_LIBRARIES})
TARGET_COMPILE_OPTIONS(${PROJECT_NAME} PUBLIC ${BOOST_FLAG} -DBLAS -DUSELAPACK -D_HAS_CBLAS -DFAST_MTP ${MKL_FLAG})

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

TARGET_COMPILE_OPTIONS(${PROJECT_NAME} BEFORE PUBLIC ${OMPFLAG} -O2 -funroll-loops -Werror -Wno-deprecated-declarations)
