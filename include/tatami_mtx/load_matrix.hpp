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
 * @cond
 */
template<bool row_, typename Data_, typename Index_, typename StoredData_, typename StoredIndex_, typename TempIndex_, typename Parser_>
std::shared_ptr<tatami::Matrix<Data_, Index_> > load_sparse_matrix_basic(Parser_& parser, eminem::Field field, size_t NR, size_t NC, size_t NL) {
    std::vector<typename std::conditional<row_, TempIndex_, StoredIndex_>::type> rows;
    std::vector<typename std::conditional<!row_, TempIndex_, StoredIndex_>::type> columns;
    rows.reserve(NL), columns.reserve(NL);
    std::vector<StoredData_> values;
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

    return std::shared_ptr<tatami::Matrix<Data_, Index_> >(
        new tatami::CompressedSparseMatrix<row_, Data_, Index_, decltype(values), decltype(indices), decltype(ptr)>(
            NR, NC, std::move(values), std::move(indices), std::move(ptr), false
        )
    );
}

template<bool row_, typename Data_, typename Index_, typename StoredData_, typename StoredIndex_, typename TempIndex_, typename Parser_>
std::shared_ptr<tatami::Matrix<Data_, Index_> > load_sparse_matrix_data(Parser_& parser, eminem::Field field, size_t NR, size_t NC, size_t NL) {
    if constexpr(std::is_same<StoredData_, Automatic>::value) {
        if (field == eminem::Field::REAL || field == eminem::Field::DOUBLE) {
            return load_sparse_matrix_basic<row_, Data_, Index_, double, StoredIndex_, TempIndex_>(parser, field, NR, NC, NL);
        }
        if (field != eminem::Field::INTEGER) {
            throw std::runtime_error("unsupported Matrix Market field type");
        }
        return load_sparse_matrix_basic<row_, Data_, Index_, int, StoredIndex_, TempIndex_>(parser, field, NR, NC, NL);
    } else {
        return load_sparse_matrix_basic<row_, Data_, Index_, StoredData_, StoredIndex_, TempIndex_>(parser, field, NR, NC, NL);
    }
}

template<bool row_, typename Data_, typename Index_, typename StoredData_, typename StoredIndex_, typename TempIndex_, typename Parser_>
std::shared_ptr<tatami::Matrix<Data_, Index_> > load_sparse_matrix_index(Parser_& parser, eminem::Field field, size_t NR, size_t NC, size_t NL) {
    if constexpr(std::is_same<StoredIndex_, Automatic>::value) {
        // Automatically choosing a smaller integer type, if it fits.
        constexpr size_t limit8 = std::numeric_limits<uint8_t>::max(), limit16 = std::numeric_limits<uint16_t>::max();
        size_t target = (row_ ? NC : NR);

        if (target <= limit8) {
            return load_sparse_matrix_data<row_, Data_, Index_, StoredData_, uint8_t, TempIndex_>(parser, field, NR, NC, NL);
        } else if (target <= limit16) {
            return load_sparse_matrix_data<row_, Data_, Index_, StoredData_, uint16_t, TempIndex_>(parser, field, NR, NC, NL);
        } else {
            return load_sparse_matrix_data<row_, Data_, Index_, StoredData_, uint32_t, TempIndex_>(parser, field, NR, NC, NL);
        }

    } else {
        return load_sparse_matrix_data<row_, Data_, Index_, StoredData_, StoredIndex_, TempIndex_>(parser, field, NR, NC, NL);
    }
}
/**
 * @endcond
 */

/**
 * @cond
 */
template<bool row_, typename Data_, typename Index_, typename StoredData_, typename Parser_>
std::shared_ptr<tatami::Matrix<Data_, Index_> > load_dense_matrix_basic(Parser_& parser, eminem::Field field, size_t NR, size_t NC) {
    std::vector<StoredData_> values;
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

    return std::shared_ptr<tatami::Matrix<Data_, Index_> >(
        new tatami::DenseMatrix<row_, Data_, Index_, decltype(values)>(NR, NC, std::move(values))
    );
}
/**
 * @endcond
 */

/**
 * Load a `tatami::Matrix` from a Matrix Market file.
 * Coordinate formats will yield a sparse matrix, while array formats will yield a dense matrix.
 * The storage types depend on the Matrix Market field type as well as the settings of `StoredData_` and `StoredIndex_`.
 *
 * @tparam row_ Whether to produce a dense row-major or compressed sparse row matrix.
 * If `false`, column-based matrices are returned instead.
 * @tparam Data_ Data type for the `tatami::Matrix` interface.
 * @tparam Index_ Integer index type for the `tatami::Matrix` interface.
 * @tparam StoredData_ Matrix data type that is stored in memory.
 * If set to `Automatic`, it defaults to `double` for real/double fields and `int` for integer fields.
 * @tparam StoredIndex_ Index data type that is stored in memory for sparse matrices.
 * If set to `Automatic`, it defaults to `uint8_t` if no dimension is greater than 255; `uint16_t` if no dimension is greater than 65536; and `int` otherwise.
 * @tparam parallel_ Whether to parallelize the loading and parsing.
 *
 * @param reader A `byteme::Reader` instance containing bytes from a Matrix Market file.
 *
 * @return Pointer to a `tatami::Matrix` instance containing data from the Matrix Market file.
 */
template<bool row_, typename Data_, typename Index_, typename StoredData_ = Automatic, typename StoredIndex_ = Automatic, bool parallel_ = false>
std::shared_ptr<tatami::Matrix<Data_, Index_> > load_matrix(byteme::Reader& reader) {
    eminem::Parser<parallel_> parser(&reader);
    parser.scan_preamble();

    const auto& banner = parser.get_banner();
    auto field = banner.field;
    size_t NR = parser.get_nrows(), NC = parser.get_ncols(), NL = parser.get_nlines();

    if (banner.format == eminem::Format::COORDINATE) {
        // Automatically choosing a smaller integer type for the temporary index.
        constexpr size_t limit8 = std::numeric_limits<uint8_t>::max(), limit16 = std::numeric_limits<uint16_t>::max();
        auto primary = (row_ ? NR : NC);

        if (primary <= limit8) {
            return load_sparse_matrix_index<row_, Data_, Index_, StoredData_, StoredIndex_, uint8_t>(parser, field, NR, NC, NL);
        } else if (primary <= limit16) {
            return load_sparse_matrix_index<row_, Data_, Index_, StoredData_, StoredIndex_, uint16_t>(parser, field, NR, NC, NL);
        } else {
            return load_sparse_matrix_index<row_, Data_, Index_, StoredData_, StoredIndex_, uint32_t>(parser, field, NR, NC, NL);
        }

    } else {
        if constexpr(std::is_same<StoredData_, Automatic>::value) {
            if (field == eminem::Field::REAL || field == eminem::Field::DOUBLE) {
                return load_dense_matrix_basic<row_, Data_, Index_, double>(parser, field, NR, NC);
            }
            if (field != eminem::Field::INTEGER) {
                throw std::runtime_error("unsupported Matrix Market field type");
            }
            return load_dense_matrix_basic<row_, Data_, Index_, int>(parser, field, NR, NC);

        } else {
            return load_dense_matrix_basic<row_, Data_, Index_, StoredData_>(parser, field, NR, NC);
        }
    }
}

/**
 * Load a `tatami::Matrix` from a (possibly Gzip-compressed) Matrix Market file, see `load_matrix()` for details.
 *
 * @tparam row_ Whether to produce a row-based matrix, see `load_matrix()` for details.
 * @tparam Data_ Data type for the `tatami::Matrix` interface.
 * @tparam Index_ Integer index type for the `tatami::Matrix` interface.
 * @tparam StoredData_ Matrix data type that is stored in memory, see `load_matrix()` for details.
 * @tparam StoredIndex_ Index data type that is stored in memory for sparse matrices,  see `load_matrix()` for details.
 * @tparam parallel_ Whether to parallelize the loading and parsing.
 *
 * @param filepath Path to a Matrix Market file.
 * The file should contain non-negative integer data in the coordinate format.
 * @param compression Compression method for the file - no compression (0) or Gzip compression (1).
 * If set to -1, the function will automatically guess the compression based on magic numbers.
 * Note that non-zero values are only supported if Zlib is available.
 * @param bufsize Size of the buffer (in bytes) to use when reading from file. 
 *
 * @return Pointer to a `tatami::Matrix` instance containing data from the Matrix Market file.
 */
template<bool row_, typename Data_, typename Index_, bool parallel_ = false, typename StoredData_ = Automatic, typename StoredIndex_ = Automatic>
std::shared_ptr<tatami::Matrix<Data_, Index_> > load_matrix_from_file(const char * filepath, int compression = 0, size_t bufsize = 65536) {
    if (compression != 0) {
#if __has_include("zlib.h")
        if (compression == -1) {
            byteme::SomeFileReader reader(filepath, bufsize);
            return load_matrix<row_, Data_, Index_, StoredData_, StoredIndex_, parallel_>(reader);
       } else if (compression == 1) {
            byteme::GzipFileReader reader(filepath, bufsize);
            return load_matrix<row_, Data_, Index_, StoredData_, StoredIndex_, parallel_>(reader);
        }
#else
        throw std::runtime_error("tatami not compiled with support for non-zero 'compression'");
#endif
    }

    byteme::RawFileReader reader(filepath, bufsize);
    return load_matrix<row_, Data_, Index_, StoredData_, StoredIndex_, parallel_>(reader);
}

/**
 * Load a `tatami::Matrix` from a buffer containing a (possibly Gzip-compressed) Matrix Market file, see `load_matrix()` for details.
 *
 * @tparam row_ Whether to produce a row-based matrix, see `load_matrix()` for details.
 * @tparam Data_ Data type for the `tatami::Matrix` interface.
 * @tparam Index_ Integer index type for the `tatami::Matrix` interface.
 * @tparam StoredData_ Matrix data type that is stored in memory, see `load_matrix()` for details.
 * @tparam StoredIndex_ Index data type that is stored in memory for sparse matrices,  see `load_matrix()` for details.
 * @tparam parallel_ Whether to parallelize the loading and parsing.
 *
 * @param buffer Array containing the contents of a Matrix Market file.
 * The file should contain non-negative integer data in the coordinate format.
 * @param n Length of the array.
 * @param compression Compression method for the file contents - no compression (0) or Gzip/Zlib compression (1).
 * If set to -1, the function will automatically guess the compression based on magic numbers.
 * Note that non-zero values are only supported if Zlib is available.
 * @param bufsize Size of the buffer (in bytes) to use when decompressing the file contents.
 * 
 * @return Pointer to a `tatami::Matrix` instance containing data from the Matrix Market file.
 */
template<bool row_, typename Data_, typename Index_, typename StoredData_ = Automatic, typename StoredIndex_ = Automatic, bool parallel_ = false>
std::shared_ptr<tatami::Matrix<Data_, Index_> > load_matrix_from_buffer(const unsigned char * buffer, size_t n, int compression = 0, size_t bufsize = 65536) {
    if (compression != 0) {
#if __has_include("zlib.h")
        if (compression == -1) {
            byteme::SomeBufferReader reader(buffer, n, bufsize);
            return load_matrix<row_, Data_, Index_, StoredData_, StoredIndex_, parallel_>(reader);
        } else if (compression == 1) {
            byteme::ZlibBufferReader reader(buffer, n, 3, bufsize);
            return load_matrix<row_, Data_, Index_, StoredData_, StoredIndex_, parallel_>(reader);
        }
#else
        throw std::runtime_error("tatami not compiled with support for non-zero 'compression'");
#endif
    }

    byteme::RawBufferReader reader(buffer, n);
    return load_matrix<row_, Data_, Index_, StoredData_, StoredIndex_, parallel_>(reader);
}

}

#endif
