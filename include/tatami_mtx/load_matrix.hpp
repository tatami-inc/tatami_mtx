#ifndef TATAMI_MTX_SIMPLE_HPP
#define TATAMI_MTX_SIMPLE_HPP

#include "tatami/tatami.hpp"
#include "eminem/eminem.hpp"
#include "byteme/byteme.hpp"

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
    size_t buffer_size = 65536;

    /**
     * Whether to parallelize the reading and parsing.
     * If true, chunks of the file are read (and decompressed) in one thread while the contents are parsed in another thread.
     */
    bool parallel = false;

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

template<bool row_, typename Value_, typename Index_, typename StoredValue_, typename StoredIndex_, typename TempIndex_, typename Parser_>
std::shared_ptr<tatami::Matrix<Value_, Index_> > load_sparse_matrix_basic(Parser_& parser, eminem::Field field, size_t NR, size_t NC, size_t NL) {
    std::vector<typename std::conditional<row_, TempIndex_, StoredIndex_>::type> rows;
    std::vector<typename std::conditional<!row_, TempIndex_, StoredIndex_>::type> columns;
    rows.reserve(NL), columns.reserve(NL);
    std::vector<StoredValue_> values;
    values.reserve(NL);

    if (field == eminem::Field::INTEGER) {
        parser.scan_integer([&](size_t r, size_t c, int v) -> void {
            values.push_back(v);
            rows.push_back(r - 1);
            columns.push_back(c - 1);
        });

    } else if (field == eminem::Field::REAL || field == eminem::Field::DOUBLE) {
        parser.scan_real([&](size_t r, size_t c, double v) -> void {
            values.push_back(v);
            rows.push_back(r - 1);
            columns.push_back(c - 1);
        });

    } else {
        throw std::runtime_error("unsupported Matrix Market field type");
    }

    auto ptr = tatami::compress_sparse_triplets<row_>(NR, NC, values, rows, columns);
    std::vector<StoredIndex_> indices;
    if constexpr(row_) {
        indices.swap(columns);
    } else {
        indices.swap(rows);
    }

    return std::shared_ptr<tatami::Matrix<Value_, Index_> >(
        new tatami::CompressedSparseMatrix<Value_, Index_, decltype(values), decltype(indices), decltype(ptr)>(
            NR, NC, std::move(values), std::move(indices), std::move(ptr), row_, false
        )
    );
}

template<bool row_, typename Value_, typename Index_, typename StoredValue_, typename StoredIndex_, typename TempIndex_, typename Parser_>
std::shared_ptr<tatami::Matrix<Value_, Index_> > load_sparse_matrix_data(Parser_& parser, eminem::Field field, size_t NR, size_t NC, size_t NL) {
    if constexpr(std::is_same<StoredValue_, Automatic>::value) {
        if (field == eminem::Field::REAL || field == eminem::Field::DOUBLE) {
            return load_sparse_matrix_basic<row_, Value_, Index_, double, StoredIndex_, TempIndex_>(parser, field, NR, NC, NL);
        }
        if (field != eminem::Field::INTEGER) {
            throw std::runtime_error("unsupported Matrix Market field type");
        }
        return load_sparse_matrix_basic<row_, Value_, Index_, int, StoredIndex_, TempIndex_>(parser, field, NR, NC, NL);
    } else {
        return load_sparse_matrix_basic<row_, Value_, Index_, StoredValue_, StoredIndex_, TempIndex_>(parser, field, NR, NC, NL);
    }
}

template<bool row_, typename Value_, typename Index_, typename StoredValue_, typename StoredIndex_, typename TempIndex_, typename Parser_>
std::shared_ptr<tatami::Matrix<Value_, Index_> > load_sparse_matrix_index(Parser_& parser, eminem::Field field, size_t NR, size_t NC, size_t NL) {
    if constexpr(std::is_same<StoredIndex_, Automatic>::value) {
        // Automatically choosing a smaller integer type, if it fits.
        constexpr size_t limit8 = std::numeric_limits<uint8_t>::max(), limit16 = std::numeric_limits<uint16_t>::max();
        size_t target = (row_ ? NC : NR);

        if (target <= limit8) {
            return load_sparse_matrix_data<row_, Value_, Index_, StoredValue_, uint8_t, TempIndex_>(parser, field, NR, NC, NL);
        } else if (target <= limit16) {
            return load_sparse_matrix_data<row_, Value_, Index_, StoredValue_, uint16_t, TempIndex_>(parser, field, NR, NC, NL);
        } else {
            return load_sparse_matrix_data<row_, Value_, Index_, StoredValue_, uint32_t, TempIndex_>(parser, field, NR, NC, NL);
        }

    } else {
        return load_sparse_matrix_data<row_, Value_, Index_, StoredValue_, StoredIndex_, TempIndex_>(parser, field, NR, NC, NL);
    }
}

template<bool row_, typename Value_, typename Index_, typename StoredValue_, typename Parser_>
std::shared_ptr<tatami::Matrix<Value_, Index_> > load_dense_matrix_basic(Parser_& parser, eminem::Field field, size_t NR, size_t NC) {
    std::vector<StoredValue_> values;
    if constexpr(row_) {
        values.resize(NR * NC);
    } else {
        values.reserve(NR * NC);
    }

    if (field == eminem::Field::INTEGER) {
        parser.scan_integer([&](size_t r, size_t c, int v) -> void {
            if constexpr(row_) {
                values[(r - 1) * NC + (c - 1)] = v;
            } else {
                values.push_back(v); // Matrix Market ARRAY format is already column-major
            }
        });

    } else if (field == eminem::Field::REAL || field == eminem::Field::DOUBLE) {
        parser.scan_real([&](size_t r, size_t c, double v) -> void {
            if constexpr(row_) {
                values[(r - 1) * NC + (c - 1)] = v;
            } else {
                values.push_back(v);
            }
        });

    } else {
        throw std::runtime_error("unsupported Matrix Market field type");
    }

    return std::shared_ptr<tatami::Matrix<Value_, Index_> >(
        new tatami::DenseMatrix<Value_, Index_, decltype(values)>(NR, NC, std::move(values), row_)
    );
}

template<bool row_, bool parallel_, typename Value_, typename Index_, typename StoredValue_, typename StoredIndex_>
std::shared_ptr<tatami::Matrix<Value_, Index_> > load_matrix(byteme::Reader& reader) {
    eminem::Parser<parallel_> parser(&reader);
    parser.scan_preamble();

    const auto& banner = parser.get_banner();
    auto field = banner.field;
    auto format = banner.format;
    size_t NR = parser.get_nrows(), NC = parser.get_ncols(), NL = parser.get_nlines();

    if (format == eminem::Format::COORDINATE) {
        // Automatically choosing a smaller integer type for the temporary index.
        constexpr size_t limit8 = std::numeric_limits<uint8_t>::max(), limit16 = std::numeric_limits<uint16_t>::max();
        auto primary = (row_ ? NR : NC);

        if (primary <= limit8) {
            return load_sparse_matrix_index<row_, Value_, Index_, StoredValue_, StoredIndex_, uint8_t>(parser, field, NR, NC, NL);
        } else if (primary <= limit16) {
            return load_sparse_matrix_index<row_, Value_, Index_, StoredValue_, StoredIndex_, uint16_t>(parser, field, NR, NC, NL);
        } else {
            return load_sparse_matrix_index<row_, Value_, Index_, StoredValue_, StoredIndex_, uint32_t>(parser, field, NR, NC, NL);
        }

    } else {
        if constexpr(std::is_same<StoredValue_, Automatic>::value) {
            if (field == eminem::Field::REAL || field == eminem::Field::DOUBLE) {
                return load_dense_matrix_basic<row_, Value_, Index_, double>(parser, field, NR, NC);
            }
            if (field != eminem::Field::INTEGER) {
                throw std::runtime_error("unsupported Matrix Market field type");
            }
            return load_dense_matrix_basic<row_, Value_, Index_, int>(parser, field, NR, NC);

        } else {
            return load_dense_matrix_basic<row_, Value_, Index_, StoredValue_>(parser, field, NR, NC);
        }
    }
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
    if (options.row) {
        if (options.parallel) {
            return internal::load_matrix<true, true, Value_, Index_, StoredValue_, StoredIndex_>(reader);
        } else {
            return internal::load_matrix<true, false, Value_, Index_, StoredValue_, StoredIndex_>(reader);
        }
    } else {
        if (options.parallel) {
            return internal::load_matrix<false, true, Value_, Index_, StoredValue_, StoredIndex_>(reader);
        } else {
            return internal::load_matrix<false, false, Value_, Index_, StoredValue_, StoredIndex_>(reader);
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
    byteme::RawFileReader reader(filepath, options.buffer_size);
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
    byteme::GzipFileReader reader(filepath, options.buffer_size);
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
    byteme::SomeFileReader reader(filepath, options.buffer_size);
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
std::shared_ptr<tatami::Matrix<Value_, Index_> > load_matrix_from_text_buffer(const unsigned char* buffer, size_t n, const Options& options) {
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
std::shared_ptr<tatami::Matrix<Value_, Index_> > load_matrix_from_zlib_buffer(const unsigned char* buffer, size_t n, const Options& options) {
    byteme::ZlibBufferReader reader(buffer, n, options.compression, options.buffer_size);
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
std::shared_ptr<tatami::Matrix<Value_, Index_> > load_matrix_from_some_buffer(const unsigned char* buffer, size_t n, const Options& options) {
    byteme::SomeBufferReader reader(buffer, n, options.buffer_size);
    return load_matrix<Value_, Index_, StoredValue_, StoredIndex_>(reader, options);
}

#endif

}

#endif
