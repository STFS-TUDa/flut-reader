#include "containers.hpp"

#include "parser/pyflut-hdf5-highfive.cpp"

#include <cerrno>
#include <chrono>
#include <cstring>
#include <filesystem> // For shorten the path
#include <iostream>
#include <numeric>
#include <set>
#include <sys/file.h>
#include <sys/mman.h>
#include <sys/stat.h> /* For mode constants */
#include <unistd.h>
#include <unordered_map>

namespace FLUT {

// *********************************** //
//
// Helper functions
//
// *********************************** //

// Checks if the entries of a vector are unique.
template <typename T>
bool isUnique(std::vector<T> v) {
    std::set<std::string> uniqueV(v.begin(), v.end());
    if (uniqueV.size() == v.size()) {
        return true;
    } else {
        return false;
    }
}

// Helper function to check if two vectors have the elements (can be different order).
// The function does not reorder the original array.
template <typename T>
bool hasSameElements(const std::vector<T> &v1, const std::vector<T> &v2) {
    // Create copies of the vectors to avoid modifying the originals
    std::vector<T> sortedV1 = v1;
    std::vector<T> sortedV2 = v2;

    // Sort the copies
    std::sort(sortedV1.begin(), sortedV1.end());
    std::sort(sortedV2.begin(), sortedV2.end());

    // Compare the sorted copies
    return sortedV1 == sortedV2;
}

template <typename T>
std::string hashString(const T &input) {
    // Use std::hash to hash the string
    std::hash<T> hasher;
    std::size_t hashValue = hasher(input);

    // Convert the hash value to a hexadecimal string
    std::stringstream ss;
    ss << std::hex << hashValue;

    return ss.str();
}

std::string getFlutname(const std::string &path) {
    std::filesystem::path fsPath(path);
    return fsPath.filename().string();
}

std::string getUsername() {
#ifdef _WIN32
    return std::getenv("USERNAME");
#else
    return std::getenv("USER");
#endif
}

std::string getTimeHash() {
    auto now = std::chrono::system_clock::now();

    // Compute duration since epoch
    auto duration = now.time_since_epoch();
    auto nanoseconds = std::chrono::duration_cast<std::chrono::nanoseconds>(duration).count();

    // Generate hash from the nanoseconds value
    return hashString<long long>(nanoseconds);
}

// *********************************** //
//
// NormalizationValues Methods
//
// *********************************** //

void DataArray::calculateDimensionOffsets() {
    /*
     * Calculates the offsets for each dimension so we can calculate the total offset into the
     * buffer
     * Array indexing in C(++) is just syntactic sugar for pointer arithmetic:
     * float* buffer = new float[10];
     * x = buffer[i]
     * x = *(&buffer + sizeof(float) * i)
     *
     * for 2 Dims: x= buffer[i][j]
     * x = *(&buffer + sizeof(float) * (i * dim1 + j))
     *
     * for 3 Dims: x = buffer[i][j][k]
     * x = *(&buffer + sizeof(float) * (i * dim1 * dim2 + j * dim2 + k))
     *
     * For arrays with a number of dimensions and dimension sizes that aren't known at compile time,
     * the compiler can't provide this abstraction, so we have to do it ourselves.
     * To make this easier we precompute the `dim1 * dim2 * ...` expressions
     *
     */
    dimensionOffsets.resize(dimensionSizes.size());
    dimensionOffsets.back() = 1;
    std::partial_sum(dimensionSizes.rbegin(), dimensionSizes.rend() - 1,
                     dimensionOffsets.rbegin() + 1, std::multiplies<>());
}

const std::vector<uint32_t> &DataArray::getDimensionOffset() {
    return dimensionOffsets;
}

const uint32_t DataArray::dimensionOffset(const size_t i) {
    return dimensionOffsets[i];
}

const size_t DataArray::dimensionSize(const size_t i) {
    return dimensionSizes[i];
}

const size_t DataArray::nVariables() {
    return dimensionSizes.back();
}

// *********************************** //
//
// InputDict Methods
//
// *********************************** //

InputDict::InputDict(size_t nVars)
    : names(nVars),
      values(nVars),
      normalizedDims(nVars),
      normSpec(nVars) {}

InputDict::InputDict(const std::string &fileName, const std::string &fileType) {
    if (fileType == "pyflut")
        *this = H5FlutParser::readInputDict(fileName);
    else
        throw std::invalid_argument("Invalid fileType: " + fileType +
                                    ". Available types: ['pyflut'].");
}

size_t InputDict::size() {
    return names.size();
}

bool InputDict::isNormalized(const uint32_t i) {
    return normalizedDims[i];
}

void InputDict::printInfo() const {
    std::cout << "  Input Variables: " << std::endl;
    std::cout << "  ( ";
    for (auto &v : names)
        std::cout << v << " ";
    std::cout << ")" << std::endl << std::endl;

    std::cout << "  Normalization settings: " << std::endl;
    std::cout << "  Name : (entry, value)" << std::endl;
    std::cout << "  ------------------------" << std::endl;

    for (const auto &m : normalizationSettings) {
        std::cout << "    " << m.first << ": ";
        for (const auto &v : m.second)
            std::cout << "(" << v.first << ": " << v.second << "), ";
        std::cout << std::endl;
    }
}

// *********************************** //
//
// OutputDict Methods
//
// *********************************** //

OutputDict::OutputDict(std::unordered_map<std::string, size_t> map) : outputDict(map) {}

OutputDict::OutputDict(const std::string &fileName, const std::string &fileType) {
    if (fileType == "pyflut")
        *this = H5FlutParser::readOutputDict(fileName);
    else
        throw std::invalid_argument("Invalid fileType: " + fileType +
                                    ". Available types: ['pyflut'].");
}

const std::vector<std::string> OutputDict::getNames() const {
    std::vector<std::string> names(outputDict.size());
    for (const auto &v : outputDict)
        names[v.second] = v.first;
    return names;
}

size_t OutputDict::getIndex(const std::string &name) {
    if (outputDict.find(name) == outputDict.end())
        throw std::invalid_argument("Requested outputVariable: '" + name +
                                    "' not present in FLUT.");
    return outputDict[name];
}

void OutputDict::containsVariables(const std::vector<std::string> &names) {
    for (auto &name : names) {
        containsVariable(name);
    }
}

void OutputDict::containsVariable(const std::string &name) {
    auto outputVariableNames = getNames();
    // Check if each element of list1 is in list2
    if (std::find(outputVariableNames.begin(), outputVariableNames.end(), name) ==
        outputVariableNames.end()) {
        throw std::invalid_argument("Requested outputVariable: '" + name +
                                    "' not present in FLUT.");
    }
}

void OutputDict::printInfo() const {
    std::cout << "  Available Output Variables: " << std::endl;
    std::cout << "  ( ";
    for (auto &v : outputDict)
        std::cout << v.first << " ";
    std::cout << ")" << std::endl;
}

// *********************************** //
//
// SharedData Methods
//
// *********************************** //

std::vector<std::string>
DataSharedMemory::defineOutputVariables(const std::string fileName,
                                        const std::vector<std::string> &outputVariables) {
    // Define outputVariablesToExtract
    // If outputVariablesToExtract is an empty vector, all outputVariables present in the hdf5-file
    // are read.
    if (!isUnique(outputVariables)) {
        throw std::invalid_argument("The requested outputVariables are not unique.");
    }

    FLUT::OutputDict outputDict(fileName, fileType);

    std::vector<std::string> variablesToExtract;
    if (outputVariables.empty()) {
        variablesToExtract = outputDict.getNames();
    } else {
        variablesToExtract = outputVariables;
    }

    outputDict.containsVariables(variablesToExtract);

    return variablesToExtract;
}

DataSharedMemory::DataSharedMemory(const std::string &fileName,
                                   const std::vector<std::string> &outputVariablesToExtract,
                                   const std::string &fileType_)
    : fileType(fileType_) {
    // Define the outputVariables.
    auto variablesToExtract = defineOutputVariables(fileName, outputVariablesToExtract);
    // Save variablesToExtract
    variableNames = variablesToExtract;

    // Get the data dimension.
    dimensionSizes = getDataDimension(fileName);

    // The last dimension of dimensionSizes is the number of outputVariables
    // Since only a subset of variables is read this needs to be overwritten.
    dimensionSizes.back() = variableNames.size();

    // Calculate the dimensionOffsets
    calculateDimensionOffsets();

    // Initialze the data buffer as a shared Memory object

    // Calculate the necessary size of the shared memory buffer.
    // Since shm_open / mmap is used this needs to be given in bytes.
    bufferSize = nElementsPerVariable() * nVariables() * sizeof(double);

    // Define name of sharedBuffer
    setBufferName(fileName, variableNames);

    // Open the shared buffer, or create it if it doesn't exist
    int fileDescriptor = shm_open(bufferName.c_str(), O_CREAT | O_RDWR, S_IRUSR | S_IWUSR);

    if (fileDescriptor == -1) {
        throw std::runtime_error("shm_open() failed for shared memory name: " + bufferName +
                                 " errno: " + std::to_string(errno) + " (" + strerror(errno) + ")");
    }

#if defined(__APPLE__)
    // macOS's shm_open() file descriptors do not support flock() at all -
    // every flock() call on one fails with ENOTSUP, confirmed empirically.
    // There is no cross-process coordination available here, so every
    // process just creates, sizes and fills its own copy of the buffer
    // instead of one process filling it once for all others to share.
    // This forgoes the memory-sharing optimisation this class is designed
    // around (multiple ranks per machine sharing one buffer) on macOS, but
    // is correct for a single process and doesn't depend on an unsupported
    // API. The later (already-failing, return-value-ignored) flock() calls
    // below are harmless no-ops in that case.
    hasExclusiveLock = true;
#else
    // Attempt to get an exclusive lock on the shared buffer
    // LOCK_NB means it doesn't block and wait until the lock is available,
    // and only returns  the success status instead
    // One process on each compute node will successfully get this lock
    // and is then responsible for filling the buffer
    // with the correct values (and later deleting it)
    // All other processes on the compute node will continue
    hasExclusiveLock = (flock(fileDescriptor, LOCK_EX | LOCK_NB) == 0);
#endif

    // Read in the data from the table.
    if (hasExclusiveLock) {
        //
        // We have to set the size of this shared memory with ftrunacte
        ftruncate(fileDescriptor, bufferSize);

        // Now we can map this fake "file" into memory
        buffer = static_cast<double *>(
            mmap(NULL, bufferSize, PROT_READ | PROT_WRITE, MAP_SHARED, fileDescriptor, 0));

        if (buffer == MAP_FAILED) {
            // Clean up the shared buffer
            close(fileDescriptor);
            shm_unlink(bufferName.c_str());
            throw std::runtime_error("Failed to mmap newly created shared memory. errno: " +
                                     std::to_string(errno));
        }

        // Read in the data from the hdf5-file.
        // The indices that are extracted are sorted prior to data extraction.
        readDataIntoBuffer(fileName);

        // Reorder data in buffer to the original order.
        // This is necessary for HighFive read in and might not be needed for other
        // file formats.
        // The functions is called here and not in the readDataIntoBuffer
        // To allow to to be in a protected namespace, since it can only be
        // executed, during the buffer initialization.
        reorderVariables(variablesToExtract);

        // We now close it again
        // It will be mapped as read-only in all other processes in the next step
        munmap(buffer, bufferSize);
        // Release the exclusive lock, allowing other processes to get a shared lock
        flock(fileDescriptor, LOCK_UN);
    }

    // Now all processes will attempt to get a _shared_ lock on the buffer
    // This will block until the creator process of the node has finished setting up the buffer and
    // releases the _exclusive_ lock
    flock(fileDescriptor, LOCK_SH);
    // And mmap the buffer as read only (it should already be setup by whoever initialized it)
    buffer =
        static_cast<double *>(mmap(NULL, bufferSize, PROT_READ, MAP_SHARED, fileDescriptor, 0));
    if (buffer == MAP_FAILED) {
        close(fileDescriptor);
        throw std::runtime_error("Failed to mmap already existing shared memory with filename: \n" +
                                 bufferName +
                                 "Check if a file already exists in /dev/shm. \n"
                                 "errno: " +
                                 std::to_string(errno));
    }
    close(fileDescriptor);
}

DataSharedMemory::~DataSharedMemory() {
    munmap(buffer, bufferSize);
    if (hasExclusiveLock)
        shm_unlink(bufferName.c_str());
}

std::vector<size_t> DataSharedMemory::getDataDimension(const std::string &fileName) {
    if (fileType == "pyflut")
        return H5FlutParser::getDataDimension(fileName);
    else
        throw std::invalid_argument("Invalid fileType: " + fileType +
                                    ". Available types: ['pyflut'].");
}

void DataSharedMemory::readDataIntoBuffer(const std::string &fileName) {
    if (fileType == "pyflut")
        return H5FlutParser::readDataIntoBuffer(fileName, this);
    else
        throw std::invalid_argument("Invalid fileType: " + fileType +
                                    ". Available types: ['pyflut'].");
}

void DataSharedMemory::setBufferName(const std::string &fileName,
                                     const std::vector<std::string> outputVariableNames) {
    // The buffer name uniquely identifies the shared memory buffer associated with a lookup table.
    // The data object is uniquely identified by:
    //
    // - the username
    // - the name of the FLUT (filename without path)
    // - the subset of output variables selected
    // - the order of the selected output variables in the buffer
    //
    // Additionally, the buffer name must be less than 255 bytes in length.
    // Therefore, some of the information are stored as hash-values.
    //
    // The buffer name consists of different elements:
    // - userName: steinhau
    // - FLUT name: FLUT.h5
    // - HASH value for the bufferSize: "159dd7e0"
    // - HASH value for the variable index order ("0_3_2"): "49decbcb39015ac4"
    //
    // Resulting in the final filename:
    // "/steinhau_FLUT.h5_159dd7e0_873f0c428b002151"
    //
    // Note: If there are problems with the uniqueness of filenames,
    // i.e. the shared-memory object cannot be allocated.
    // The filename can additionally include a timeHash.
    // Just add timeHash() to the bufferName.

    std::string flutName = getFlutname(fileName);
    std::string userName = getUsername();

    std::string outputVariablesString;
    // Append indices to the buffer name
    for (auto &name : outputVariableNames) {
        outputVariablesString += name;
    }
    std::string ouputVariablesHash = hashString<std::string>(outputVariablesString);

    std::string readableName = "/" + userName + "_" + flutName + "_" +
                               hashString<size_t>(bufferSize) + "_" + ouputVariablesHash;

#if defined(__APPLE__)
    // macOS has no flock() coordination for shm_open() fds (see hasExclusiveLock
    // above), so every process fills its own private copy instead of one process
    // filling a buffer shared by all. That only works if the buffer name is
    // actually unique per-process - without this, multiple ranks on the same
    // machine all open/fill/reorder the SAME named segment concurrently with no
    // synchronization, a torn-write race that corrupts the table (confirmed:
    // produced nonphysical field values like ha=1e12 during a parallel
    // flameletFoam run). Appending the PID gives each process a genuinely
    // private buffer.
    readableName += "_" + std::to_string(getpid());
#endif

    // POSIX shared memory names are subject to platform-specific length limits
    // that can be much stricter than a regular filesystem's NAME_MAX. Notably,
    // macOS restricts shm_open() names to 31 bytes total (including the leading
    // '/'), confirmed empirically (shm_open() fails with ENAMETOOLONG above that
    // length) - there is no public header constant for it. Linux's tmpfs-backed
    // implementation allows names up to NAME_MAX, so keep the readable name
    // there and only fall back to a fully-hashed (and thus always-short) name
    // when the readable form would not fit.
#if defined(__APPLE__)
    constexpr std::size_t maxShmNameLength = 31;
#else
    constexpr std::size_t maxShmNameLength = NAME_MAX;
#endif

    if (readableName.size() <= maxShmNameLength) {
        bufferName = readableName;
    } else {
        bufferName = "/" + hashString<std::string>(readableName);
    }

    if (bufferName.size() > maxShmNameLength) {
        throw std::runtime_error("Shared memory name too long: " + bufferName);
    }
}

void DataSharedMemory::reorderVariables(const std::vector<std::string> &names) {
    if (variableNames.size() != names.size())
        throw std::invalid_argument(
            "The number of variables must be similar to the variables in the buffer.");

    if (!hasSameElements<std::string>(variableNames, names))
        throw std::invalid_argument(
            "The names in the given vector of strings does not match the variable names in the "
            "data buffer.");

    // Sort the extracted data according to the original order of the requested outputVariables.
    // This action is done inplace to avoid double memory usage.
    for (int i = 0; i < variableNames.size(); i++) {
        std::string name = names[i];
        int current_index =
            std::find(variableNames.begin(), variableNames.end(), name) - variableNames.begin();
        int desired_index = i;

        if (current_index != desired_index) {
            for (int j = 0; j < nElementsPerVariable(); j++) {
                size_t index1 = current_index + (nVariables()) * j;
                size_t index2 = desired_index + (nVariables()) * j;
                // Swap the elements if they are not the same
                if (index1 != index2) {
                    std::swap(buffer[index1], buffer[index2]);
                }
            }
            // Swap the elements in the outputVariableVector.
            std::swap(variableNames[desired_index], variableNames[current_index]);
        }
    }
}

//
// Getter / setter methods
//
size_t DataSharedMemory::nElementsPerVariable() {
    return std::accumulate(dimensionSizes.begin(), dimensionSizes.end() - 1, static_cast<size_t>(1),
                           std::multiplies<size_t>());
}

void DataSharedMemory::printInfo() const {
    std::cout << "  Shared buffer name: " << bufferName << std::endl;

    std::cout << "  dataDimension: (";
    for (auto &v : dimensionSizes)
        std::cout << v << ", ";
    std::cout << ")" << std::endl;

    std::cout << "  Loaded variables: (" << std::endl;
    for (auto &v : variableNames)
        std::cout << v << " ";
    std::cout << ")" << std::endl;
}

} // namespace FLUT