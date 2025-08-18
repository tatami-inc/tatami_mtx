#ifndef TATAMI_MTX_SIMPLE_HPP
#define TATAMI_MTX_SIMPLE_HPP

#include <cstddef>
#include <vector>
#include <limits>
#include <type_traits>
#include <cstdint>

#include "tatami/tatami.hpp"
#include "eminem/eminem.hpp"
#include "byteme/byteme.hpp"
#include "sanisizer/sanisizer.hpp"

/**
 * @file load_matrix.hpp
 * @brief Load a **tatami** matrix from a Matrix Market file.
 */

namespace tatami_mtx {

/**
 * @brief Enable automatic type determination.
 *
 * This is used as a placeholder type in the template arguments of `load_matrix()` to indicate that the storage types should be automatically chosen.
 */
struct Automatic {};

/**
 * @brief Options for `load_matrix()` and friends.
 */
struct Options {
    /**
     * Whether to produce a dense row-major or compressed sparse row matrix.
     * If false, column-based matrices are returned instead.
     */
    bool row = true;

    /**
     * Size of the buffer (in bytes) to use when reading the file contents.
     * This buffer size is also used for Gzip/Zlib decompression.
     * Ignored for `load_matrix_from_text_buffer()`.
     */
    std::size_t buffer_size = sanisizer::cap<std::size_t>(65536);

    /**
     * Number of threads to use for Matrix Market parsing.
     * If greater than 1, chunks of the file are read (and decompressed) in one thread while the contents are parsed in another thread.
     */
    int num_threads = 1;

    /**
     * Compression of a Zlib-compressed buffer in `load_matrix_from_zlib_buffer()`.
     * The default of 3 will auto-detect the compression method, see `byteme::ZlibBufferReader` for details.
     */
    int compression = 3;
};

/**
 * @cond
 */
namespace internal {

template<typename Value_, typename Index_, typename StoredValue_, typename StoredIndex_, typename TempIndex_, typename Parser_>
std::shared_ptr<tatami::Matrix<Value_, Index_> > load_sparse_matrix_basic(Parser_& parser, eminem::Field field, Index_ NR, Index_ NC, eminem::LineIndex NL, bool row) {
    std::vector<TempIndex_> primary;
    primary.reserve(NL);
    std::vector<StoredIndex_> secondary;
    secondary.reserve(NL);
    std::vector<StoredValue_> values;
    values.reserve(NL);

    if (field == eminem::Field::INTEGER) {
        typedef typename std::conditional<std::is_integral<StoredValue_>::value, StoredValue_, int>::type ParseType;
        if (row) {
            parser.template scan_integer<ParseType>([&](Index_ r, Index_ c, ParseType v) -> void {
                values.push_back(v);
                primary.push_back(r - 1);
                secondary.push_back(c - 1);
            });
        } else {
            parser.template scan_integer<ParseType>([&](Index_ r, Index_ c, ParseType v) -> void {
                values.push_back(v);
                primary.push_back(c - 1);
                secondary.push_back(r - 1);
            });
        }

    } else if (field == eminem::Field::REAL || field == eminem::Field::DOUBLE) {
        typedef typename std::conditional<std::is_floating_point<StoredValue_>::value, StoredValue_, double>::type ParseType;
        if (row) {
            parser.template scan_real<ParseType>([&](Index_ r, Index_ c, ParseType v) -> void {
                values.push_back(v);
                primary.push_back(r - 1);
                secondary.push_back(c - 1);
            });
        } else {
            parser.template scan_real<ParseType>([&](Index_ r, Index_ c, ParseType v) -> void {
                values.push_back(v);
                primary.push_back(c - 1);
                secondary.push_back(r - 1);
            });
        }

    } else {
        throw std::runtime_error("unsupported Matrix Market field type");
    }

    auto indptr = tatami::compress_sparse_triplets((row ? NR : NC), values, primary, secondary);
    return std::shared_ptr<tatami::Matrix<Value_, Index_> >(
        new tatami::CompressedSparseMatrix<Value_, Index_, decltype(values), decltype(secondary), decltype(indptr)>(
            NR, NC, std::move(values), std::move(secondary), std::move(indptr), row, false
        )
    );
}

template<typename Value_, typename Index_, typename StoredValue_, typename StoredIndex_, typename TempIndex_, typename Parser_>
std::shared_ptr<tatami::Matrix<Value_, Index_> > load_sparse_matrix_data(Parser_& parser, eminem::Field field, Index_ NR, Index_ NC, eminem::LineIndex NL, bool row) {
    if constexpr(std::is_same<StoredValue_, Automatic>::value) {
        if (field == eminem::Field::REAL || field == eminem::Field::DOUBLE) {
            return load_sparse_matrix_basic<Value_, Index_, double, StoredIndex_, TempIndex_>(parser, field, NR, NC, NL, row);
        }
        if (field != eminem::Field::INTEGER) {
            throw std::runtime_error("unsupported Matrix Market field type");
        }
        return load_sparse_matrix_basic<Value_, Index_, int, StoredIndex_, TempIndex_>(parser, field, NR, NC, NL, row);
    } else {
        return load_sparse_matrix_basic<Value_, Index_, StoredValue_, StoredIndex_, TempIndex_>(parser, field, NR, NC, NL, row);
    }
}

template<typename Value_, typename Index_, typename StoredValue_, typename StoredIndex_, typename TempIndex_, typename Parser_>
std::shared_ptr<tatami::Matrix<Value_, Index_> > load_sparse_matrix_index(Parser_& parser, eminem::Field field, Index_ NR, Index_ NC, eminem::LineIndex NL, bool row) {
    if constexpr(std::is_same<StoredIndex_, Automatic>::value) {
        // Automatically choosing a smaller integer type, if it fits.
        constexpr Index_ limit8 = std::numeric_limits<std::uint8_t>::max(), limit16 = std::numeric_limits<std::uint16_t>::max();
        auto target = (row ? NC : NR);

        if (target <= limit8) {
            return load_sparse_matrix_data<Value_, Index_, StoredValue_, std::uint8_t, TempIndex_>(parser, field, NR, NC, NL, row);
        } else if (target <= limit16) {
            return load_sparse_matrix_data<Value_, Index_, StoredValue_, std::uint16_t, TempIndex_>(parser, field, NR, NC, NL, row);
        } else {
            return load_sparse_matrix_data<Value_, Index_, StoredValue_, std::uint32_t, TempIndex_>(parser, field, NR, NC, NL, row);
        }

    } else {
        return load_sparse_matrix_data<Value_, Index_, StoredValue_, StoredIndex_, TempIndex_>(parser, field, NR, NC, NL, row);
    }
}

template<typename Value_, typename Index_, typename StoredValue_, typename Parser_>
std::shared_ptr<tatami::Matrix<Value_, Index_> > load_dense_matrix_basic(Parser_& parser, eminem::Field field, Index_ NR, Index_ NC, bool row) {
    std::vector<StoredValue_> values;
    auto full_size = sanisizer::product<decltype(values.size())>(NR, NC);
    if (row) {
        values.resize(full_size);
    } else {
        values.reserve(full_size);
    }

    if (field == eminem::Field::INTEGER) {
        typedef typename std::conditional<std::is_integral<StoredValue_>::value, StoredValue_, int>::type ParseType;
        if (row) {
            parser.template scan_integer<ParseType>([&](Index_ r, Index_ c, ParseType v) -> void { values[sanisizer::nd_offset<decltype(values.size())>(c - 1, NC, r - 1)] = v; });
        } else {
            parser.template scan_integer<ParseType>([&](Index_, Index_, ParseType v) -> void { values.push_back(v); }); // Matrix Market ARRAY format is already column-major
        }

    } else if (field == eminem::Field::REAL || field == eminem::Field::DOUBLE) {
        typedef typename std::conditional<std::is_floating_point<StoredValue_>::value, StoredValue_, double>::type ParseType;
        if (row) {
            parser.template scan_real<ParseType>([&](Index_ r, Index_ c, ParseType v) -> void { values[sanisizer::nd_offset<decltype(values.size())>(c - 1, NC, r - 1)] = v; });
        } else {
            parser.template scan_real<ParseType>([&](Index_, Index_, ParseType v) -> void { values.push_back(v); });
        }

    } else {
        throw std::runtime_error("unsupported Matrix Market field type");
    }

    return std::shared_ptr<tatami::Matrix<Value_, Index_> >(
        new tatami::DenseMatrix<Value_, Index_, decltype(values)>(NR, NC, std::move(values), row)
    );
}

}
/**
 * @endcond
 */

/**
 * Load a `tatami::Matrix` from a Matrix Market file.
 * Coordinate formats will yield a sparse matrix, while array formats will yield a dense matrix.
 * The storage types depend on the Matrix Market field type as well as the settings of `StoredValue_` and `StoredIndex_`.
 *
 * @tparam Value_ Data type for the `tatami::Matrix` interface.
 * @tparam Index_ Integer index type for the `tatami::Matrix` interface.
 * @tparam StoredValue_ Matrix data type that is stored in memory.
 * If set to `Automatic`, it defaults to `double` for real/double fields and `int` for integer fields.
 * @tparam StoredIndex_ Index data type that is stored in memory for sparse matrices.
 * If set to `Automatic`, it defaults to `uint8_t` if no dimension is greater than 255; `uint16_t` if no dimension is greater than 65536; and `int` otherwise.
 *
 * @param reader A `byteme::Reader` instance containing bytes from a Matrix Market file.
 * @param options Options for loading the matrix.
 *
 * @return Pointer to a `tatami::Matrix` instance containing data from the Matrix Market file.
 */
template<typename Value_, typename Index_, typename StoredValue_ = Automatic, typename StoredIndex_ = Automatic>
std::shared_ptr<tatami::Matrix<Value_, Index_> > load_matrix(byteme::Reader& reader, const Options& options) {
    byteme::PerByteSerial<char, byteme::Reader*> pb(&reader);
    eminem::Parser<decltype(&pb), Index_> parser(&pb, [&]{
        eminem::ParserOptions eopt;
        eopt.num_threads = options.num_threads;
        return eopt;
    }());

    parser.scan_preamble();
    const auto& banner = parser.get_banner();
    auto field = banner.field;
    auto format = banner.format;
    auto NR = parser.get_nrows(), NC = parser.get_ncols();
    auto NL = parser.get_nlines();

    if (format == eminem::Format::COORDINATE) {
        // Automatically choosing a smaller integer type for the temporary index.
        constexpr auto limit8 = std::numeric_limits<uint8_t>::max();
        constexpr auto limit16 = std::numeric_limits<std::uint16_t>::max();
        auto primary = (options.row ? NR : NC);

        if (sanisizer::is_less_than_or_equal(primary, limit8)) {
            return internal::load_sparse_matrix_index<Value_, Index_, StoredValue_, StoredIndex_, std::uint8_t>(parser, field, NR, NC, NL, options.row);
        } else if (sanisizer::is_less_than_or_equal(primary, limit16)) {
            return internal::load_sparse_matrix_index<Value_, Index_, StoredValue_, StoredIndex_, std::uint16_t>(parser, field, NR, NC, NL, options.row);
        } else {
            return internal::load_sparse_matrix_index<Value_, Index_, StoredValue_, StoredIndex_, std::uint32_t>(parser, field, NR, NC, NL, options.row);
        }

    } else {
        if constexpr(std::is_same<StoredValue_, Automatic>::value) {
            if (field == eminem::Field::REAL || field == eminem::Field::DOUBLE) {
                return internal::load_dense_matrix_basic<Value_, Index_, double>(parser, field, NR, NC, options.row);
            }
            if (field != eminem::Field::INTEGER) {
                throw std::runtime_error("unsupported Matrix Market field type");
            }
            return internal::load_dense_matrix_basic<Value_, Index_, int>(parser, field, NR, NC, options.row);

        } else {
            return internal::load_dense_matrix_basic<Value_, Index_, StoredValue_>(parser, field, NR, NC, options.row);
        }
    }
}

/**
 * Load a `tatami::Matrix` from a Matrix Market text file, see `load_matrix()` for details.
 *
 * @tparam Value_ Data type for the `tatami::Matrix` interface.
 * @tparam Index_ Integer index type for the `tatami::Matrix` interface.
 * @tparam StoredValue_ Matrix data type that is stored in memory, see `load_matrix()` for details.
 * @tparam StoredIndex_ Index data type that is stored in memory for sparse matrices,  see `load_matrix()` for details.
 * @tparam parallel_ Whether to parallelize the loading and parsing.
 *
 * @param filepath Path to a Matrix Market file.
 * @param options Options for loading the matrix.
 *
 * @return Pointer to a `tatami::Matrix` instance containing data from the Matrix Market file.
 */
template<typename Value_, typename Index_, typename StoredValue_ = Automatic, typename StoredIndex_ = Automatic>
std::shared_ptr<tatami::Matrix<Value_, Index_> > load_matrix_from_text_file(const char* filepath, const Options& options) {
    byteme::RawFileReader reader(filepath, [&]{
        byteme::RawFileReaderOptions opt;
        opt.buffer_size = options.buffer_size;
        return opt;
    }());
    return load_matrix<Value_, Index_, StoredValue_, StoredIndex_>(reader, options);
}

#if __has_include("zlib.h")

/**
 * Load a `tatami::Matrix` from a Gzip-compressed Matrix Market file, see `load_matrix()` for details.
 *
 * @tparam Value_ Data type for the `tatami::Matrix` interface.
 * @tparam Index_ Integer index type for the `tatami::Matrix` interface.
 * @tparam StoredValue_ Matrix data type that is stored in memory, see `load_matrix()` for details.
 * @tparam StoredIndex_ Index data type that is stored in memory for sparse matrices,  see `load_matrix()` for details.
 *
 * @param filepath Path to a Matrix Market file.
 * @param options Options for loading the matrix.
 *
 * @return Pointer to a `tatami::Matrix` instance containing data from the Matrix Market file.
 */
template<typename Value_, typename Index_, typename StoredValue_ = Automatic, typename StoredIndex_ = Automatic>
std::shared_ptr<tatami::Matrix<Value_, Index_> > load_matrix_from_gzip_file(const char* filepath, const Options& options) {
    byteme::GzipFileReader reader(filepath, [&]{
        byteme::GzipFileReaderOptions opt;
        opt.buffer_size = options.buffer_size;
        return opt;
    }());
    return load_matrix<Value_, Index_, StoredValue_, StoredIndex_>(reader, options);
}

/**
 * Load a `tatami::Matrix` from a possibly Gzip-compressed Matrix Market file, see `load_matrix()` for details.
 *
 * @tparam Value_ Data type for the `tatami::Matrix` interface.
 * @tparam Index_ Integer index type for the `tatami::Matrix` interface.
 * @tparam StoredValue_ Matrix data type that is stored in memory, see `load_matrix()` for details.
 * @tparam StoredIndex_ Index data type that is stored in memory for sparse matrices,  see `load_matrix()` for details.
 *
 * @param filepath Path to a Matrix Market file.
 * @param options Options for loading the matrix.
 *
 * @return Pointer to a `tatami::Matrix` instance containing data from the Matrix Market file.
 */
template<typename Value_, typename Index_, typename StoredValue_ = Automatic, typename StoredIndex_ = Automatic>
std::shared_ptr<tatami::Matrix<Value_, Index_> > load_matrix_from_some_file(const char* filepath, const Options& options) {
    byteme::SomeFileReader reader(filepath, [&]{
        byteme::SomeFileReaderOptions opt;
        opt.buffer_size = options.buffer_size;
        return opt;
    }());
    return load_matrix<Value_, Index_, StoredValue_, StoredIndex_>(reader, options);
}

#endif

/**
 * Load a `tatami::Matrix` from a buffer containing a Matrix Market file, see `load_matrix()` for details.
 *
 * @tparam Value_ Data type for the `tatami::Matrix` interface.
 * @tparam Index_ Integer index type for the `tatami::Matrix` interface.
 * @tparam StoredValue_ Matrix data type that is stored in memory, see `load_matrix()` for details.
 * @tparam StoredIndex_ Index data type that is stored in memory for sparse matrices,  see `load_matrix()` for details.
 *
 * @param buffer Array containing the contents of an uncompressed Matrix Market file.
 * @param n Length of the array.
 * @param options Options for loading the matrix.
 * 
 * @return Pointer to a `tatami::Matrix` instance containing data from the Matrix Market file.
 */
template<typename Value_, typename Index_, typename StoredValue_ = Automatic, typename StoredIndex_ = Automatic>
std::shared_ptr<tatami::Matrix<Value_, Index_> > load_matrix_from_text_buffer(const unsigned char* buffer, std::size_t n, const Options& options) {
    byteme::RawBufferReader reader(buffer, n);
    return load_matrix<Value_, Index_, StoredValue_, StoredIndex_>(reader, options);
}

#if __has_include("zlib.h")

/**
 * Load a `tatami::Matrix` from a buffer containing a Gzip/Zlib-compressed Matrix Market file, see `load_matrix()` for details.
 *
 * @tparam Value_ Data type for the `tatami::Matrix` interface.
 * @tparam Index_ Integer index type for the `tatami::Matrix` interface.
 * @tparam StoredValue_ Matrix data type that is stored in memory, see `load_matrix()` for details.
 * @tparam StoredIndex_ Index data type that is stored in memory for sparse matrices,  see `load_matrix()` for details.
 *
 * @param buffer Array containing the contents of a Matrix Market file after Gzip/Zlib compression.
 * @param n Length of the array.
 * @param options Options for loading the matrix.
 * 
 * @return Pointer to a `tatami::Matrix` instance containing data from the Matrix Market file.
 */
template<typename Value_, typename Index_, typename StoredValue_ = Automatic, typename StoredIndex_ = Automatic>
std::shared_ptr<tatami::Matrix<Value_, Index_> > load_matrix_from_zlib_buffer(const unsigned char* buffer, std::size_t n, const Options& options) {
    byteme::ZlibBufferReader reader(buffer, n, [&]{
        byteme::ZlibBufferReaderOptions opt;
        opt.mode = options.compression;
        opt.buffer_size = options.buffer_size;
        return opt;
    }());
    return load_matrix<Value_, Index_, StoredValue_, StoredIndex_>(reader, options);
}

/**
 * Load a `tatami::Matrix` from a buffer containing a possibly Gzip/Zlib-compressed Matrix Market file, see `load_matrix()` for details.
 *
 * @tparam Value_ Data type for the `tatami::Matrix` interface.
 * @tparam Index_ Integer index type for the `tatami::Matrix` interface.
 * @tparam StoredValue_ Matrix data type that is stored in memory, see `load_matrix()` for details.
 * @tparam StoredIndex_ Index data type that is stored in memory for sparse matrices,  see `load_matrix()` for details.
 *
 * @param buffer Array containing the contents of a Matrix Market file, possibly after Gzip/Zlib compression.
 * @param n Length of the array.
 * @param options Options for loading the matrix.
 * 
 * @return Pointer to a `tatami::Matrix` instance containing data from the Matrix Market file.
 */
template<typename Value_, typename Index_, typename StoredValue_ = Automatic, typename StoredIndex_ = Automatic>
std::shared_ptr<tatami::Matrix<Value_, Index_> > load_matrix_from_some_buffer(const unsigned char* buffer, std::size_t n, const Options& options) {
    byteme::SomeBufferReader reader(buffer, n, [&]{
        byteme::SomeBufferReaderOptions opt;
        opt.buffer_size = options.buffer_size;
        return opt;
    }());
    return load_matrix<Value_, Index_, StoredValue_, StoredIndex_>(reader, options);
}

#endif

}

#endif
