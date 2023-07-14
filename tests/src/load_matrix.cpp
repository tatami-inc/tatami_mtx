#include <gtest/gtest.h>

#include "byteme/byteme.hpp"
#include "byteme/temp_file_path.hpp"
#include "eminem/eminem.hpp"
#include "tatami_mtx/tatami_mtx.hpp"

#include <limits>
#include <fstream>
#include <string>
#include <vector>
#include <random>
#include <type_traits>

template<class T>
class LoadMatrixTestMethods {
protected:
    size_t NR, NC;
    std::vector<int> rows, cols;
    std::vector<T> vals;
    std::shared_ptr<tatami::NumericMatrix> ref;

protected:
    void initialize(size_t seed, size_t nr, size_t nc) {
        NR = nr;
        NC = nc;

        std::mt19937_64 rng(seed);
        std::uniform_real_distribution<> unif(0.0, 1.0);
        for (size_t i = 0; i < NC; ++i) {
            for (size_t j = 0; j < NR; ++j) {
                if (unif(rng) < 0.1) {
                    rows.push_back(j);
                    cols.push_back(i);
                    if constexpr(std::numeric_limits<T>::is_integer) {
                        vals.push_back(unif(rng) * 100);
                    } else {
                        vals.push_back(std::round(unif(rng) * 1000)/100); // avoid problems with precision.
                    }
                }
            }
        }

        auto indptrs = tatami::compress_sparse_triplets<false>(NR, NC, vals, rows, cols);
        typedef tatami::CompressedSparseColumnMatrix<double, int, decltype(vals), decltype(rows), decltype(indptrs)> SparseMat; 
        ref.reset(new SparseMat(NR, NC, vals, rows, std::move(indptrs))); 
    }

    void write_coordinate(byteme::Writer& stream) const {
        stream.write("%%MatrixMarket matrix coordinate ");
        if constexpr(std::numeric_limits<T>::is_integer) {
            stream.write("integer");
        } else {
            stream.write("real");
        }
        stream.write(" general\n");
        stream.write(std::to_string(NR));
        stream.write(' ');
        stream.write(std::to_string(NC));
        stream.write(' ');
        stream.write(std::to_string(vals.size()));

        for (size_t i = 0; i < vals.size(); ++i) {
            stream.write('\n');
            stream.write(std::to_string(rows[i] + 1));
            stream.write(' ');
            stream.write(std::to_string(cols[i] + 1));
            stream.write(' ');
            stream.write(std::to_string(vals[i]));
        }
        stream.write('\n');

        stream.finish();
        return;
    }

    void write_array(byteme::Writer& stream) const {
        stream.write("%%MatrixMarket matrix array ");
        if constexpr(std::numeric_limits<T>::is_integer) {
            stream.write("integer");
        } else {
            stream.write("real");
        }
        stream.write(" general\n");
        stream.write(std::to_string(NR));
        stream.write(' ');
        stream.write(std::to_string(NC));

        auto ext = ref->dense_column();
        for (size_t c = 0; c < NC; ++c) {
            auto col = ext->fetch(c);
            for (size_t r = 0; r < NR; ++r) {
                stream.write('\n');
                stream.write(std::to_string(static_cast<T>(col[r])));
            }
        }
        stream.write('\n');

        stream.finish();
        return;
    }
};

/***************************************
 *** Checking the input source types ***
 ***************************************/

class LoadMatrixInputTest : public LoadMatrixTestMethods<int>, public ::testing::Test {};

TEST_F(LoadMatrixInputTest, SimpleBuffer) {
    initialize(99, 100, 200);

    byteme::RawBufferWriter writer;
    write_coordinate(writer);
    const auto& buffer = writer.output;

    // Uncompressed.
    {
        auto out = tatami_mtx::load_matrix_from_text_buffer<false, double, int>(buffer.data(), buffer.size());
        EXPECT_FALSE(out->prefer_rows());
        EXPECT_TRUE(out->sparse());

        auto owrk = out->dense_column();
        auto rwrk = ref->dense_column();
        for (size_t i = 0; i < NC; ++i) {
            auto stuff = owrk->fetch(i);
            EXPECT_EQ(stuff, rwrk->fetch(i));
        }
    }

    // Automatic.
    {
        auto out = tatami_mtx::load_matrix_from_some_buffer<false, double, int>(buffer.data(), buffer.size());
        EXPECT_FALSE(out->prefer_rows());
        EXPECT_TRUE(out->sparse());

        auto owrk = out->dense_column();
        auto rwrk = ref->dense_column();
        for (size_t i = 0; i < NC; ++i) {
            auto stuff = owrk->fetch(i);
            EXPECT_EQ(stuff, rwrk->fetch(i));
        }
    }
}

TEST_F(LoadMatrixInputTest, SimpleText) {
    initialize(100, 99, 59);

    auto path = byteme::temp_file_path("tatami-tests-ext-MatrixMarket", ".mtx");
    byteme::RawFileWriter writer(path.c_str());
    write_coordinate(writer);

    // Uncompressed.
    {
        auto out = tatami_mtx::load_matrix_from_text_file<true, double, int>(path.c_str());
        EXPECT_TRUE(out->prefer_rows());
        EXPECT_TRUE(out->sparse());

        auto owrk = out->dense_column();
        auto rwrk = ref->dense_column();
        for (size_t i = 0; i < NC; ++i) {
            auto stuff = owrk->fetch(i);
            EXPECT_EQ(stuff, rwrk->fetch(i));
        }
    }

    // Automatic.
    {
        auto out = tatami_mtx::load_matrix_from_some_file<true, double, int>(path.c_str());
        EXPECT_TRUE(out->prefer_rows());
        EXPECT_TRUE(out->sparse());

        auto owrk = out->dense_column();
        auto rwrk = ref->dense_column();
        for (size_t i = 0; i < NC; ++i) {
            auto stuff = owrk->fetch(i);
            EXPECT_EQ(stuff, rwrk->fetch(i));
        }
    }
}

TEST_F(LoadMatrixInputTest, ZlibBuffer) {
    initialize(101, 205, 80);

    byteme::ZlibBufferWriter writer(1);
    write_coordinate(writer);
    const auto& buffer = writer.output;

    // Compressed.
    {
        auto out = tatami_mtx::load_matrix_from_zlib_buffer<false, double, int>(buffer.data(), buffer.size());
        EXPECT_FALSE(out->prefer_rows());
        EXPECT_TRUE(out->sparse());

        auto owrk = out->dense_column();
        auto rwrk = ref->dense_column();
        for (size_t i = 0; i < NC; ++i) {
            auto stuff = owrk->fetch(i);
            EXPECT_EQ(stuff, rwrk->fetch(i));
        }
    }

    // Automatic.
    {
        auto out = tatami_mtx::load_matrix_from_some_buffer<false, double, int>(buffer.data(), buffer.size());
        EXPECT_FALSE(out->prefer_rows());
        EXPECT_TRUE(out->sparse());

        auto owrk = out->dense_column();
        auto rwrk = ref->dense_column();
        for (size_t i = 0; i < NC; ++i) {
            auto stuff = owrk->fetch(i);
            EXPECT_EQ(stuff, rwrk->fetch(i));
        }
    }
}

TEST_F(LoadMatrixInputTest, GzipFile) {
    initialize(102, 200, 100);

    auto path = byteme::temp_file_path("tatami-tests-ext-MatrixMarket", ".mtx.gz");
    byteme::GzipFileWriter writer(path.c_str());
    write_coordinate(writer);

    // Compressed.
    {
        auto out = tatami_mtx::load_matrix_from_gzip_file<true, double, int>(path.c_str());
        EXPECT_TRUE(out->prefer_rows());
        EXPECT_TRUE(out->sparse());

        auto owrk = out->dense_column();
        auto rwrk = ref->dense_column();
        for (size_t i = 0; i < NC; ++i) {
            auto stuff = owrk->fetch(i);
            EXPECT_EQ(stuff, rwrk->fetch(i));
        }
    }

    // Automatic.
    {
        auto out = tatami_mtx::load_matrix_from_some_file<true, double, int>(path.c_str());
        EXPECT_TRUE(out->prefer_rows());
        EXPECT_TRUE(out->sparse());

        auto owrk = out->dense_column();
        auto rwrk = ref->dense_column();
        for (size_t i = 0; i < NC; ++i) {
            auto stuff = owrk->fetch(i);
            EXPECT_EQ(stuff, rwrk->fetch(i));
        }
    }
}

/*********************************************
 *** Checking correct choice of index type ***
 *********************************************/

class LoadMatrixIndexTest : public LoadMatrixTestMethods<int>, public ::testing::Test {};

TEST_F(LoadMatrixIndexTest, Index8) {
    initialize(1000, 240, 210); // allow automatic 8-bit representation of row indices for CSC matrix.

    byteme::RawBufferWriter writer;
    write_coordinate(writer);
    const auto& buffer = writer.output;

    auto out = tatami_mtx::load_matrix_from_text_buffer<false, double, int>(buffer.data(), buffer.size());
    EXPECT_FALSE(out->prefer_rows());
    EXPECT_TRUE(out->sparse());

    auto owrk = out->dense_column();
    auto rwrk = ref->dense_column();
    for (size_t i = 0; i < NC; ++i) {
        auto stuff = owrk->fetch(i);
        EXPECT_EQ(stuff, rwrk->fetch(i));
    }
}

TEST_F(LoadMatrixIndexTest, Index16) {
    initialize(1001, 2000, 10); // force automatic 16-bit representation

    byteme::RawBufferWriter writer;
    write_coordinate(writer);
    const auto& buffer = writer.output;

    auto out = tatami_mtx::load_matrix_from_text_buffer<false, double, int>(buffer.data(), buffer.size());
    EXPECT_FALSE(out->prefer_rows());
    EXPECT_TRUE(out->sparse());

    auto owrk = out->dense_column();
    auto rwrk = ref->dense_column();
    for (size_t i = 0; i < NC; ++i) {
        auto stuff = owrk->fetch(i);
        EXPECT_EQ(stuff, rwrk->fetch(i));
    }
}

TEST_F(LoadMatrixIndexTest, Index32) {
    initialize(1002, 100000, 2); // force automatic 32-bit representation

    byteme::RawBufferWriter writer;
    write_coordinate(writer);
    const auto& buffer = writer.output;

    auto out = tatami_mtx::load_matrix_from_text_buffer<false, double, int>(buffer.data(), buffer.size());
    EXPECT_FALSE(out->prefer_rows());
    EXPECT_TRUE(out->sparse());

    auto owrk = out->dense_column();
    auto rwrk = ref->dense_column();
    for (size_t i = 0; i < NC; ++i) {
        auto stuff = owrk->fetch(i);
        EXPECT_EQ(stuff, rwrk->fetch(i));
    }
}

TEST_F(LoadMatrixIndexTest, IndexCustom) {
    initialize(1003, 100, 200); 

    byteme::RawBufferWriter writer;
    write_coordinate(writer);
    const auto& buffer = writer.output;

    auto out = tatami_mtx::load_matrix_from_text_buffer<false, double, int, tatami_mtx::Automatic, uint32_t>(buffer.data(), buffer.size());
    EXPECT_FALSE(out->prefer_rows());
    EXPECT_TRUE(out->sparse());

    auto owrk = out->dense_column();
    auto rwrk = ref->dense_column();
    for (size_t i = 0; i < NC; ++i) {
        auto stuff = owrk->fetch(i);
        EXPECT_EQ(stuff, rwrk->fetch(i));
    }
}

/*********************************************
 *** Checking temporary index type choices ***
 *********************************************/

TEST_F(LoadMatrixIndexTest, TempIndex8) {
    initialize(2000, 10, 2000); // use 8-bit temporary indices for CSR matrix.

    byteme::RawBufferWriter writer;
    write_coordinate(writer);
    const auto& buffer = writer.output;

    auto out = tatami_mtx::load_matrix_from_text_buffer<true, double, int>(buffer.data(), buffer.size());
    EXPECT_TRUE(out->prefer_rows());
    EXPECT_TRUE(out->sparse());

    auto owrk = out->dense_column();
    auto rwrk = ref->dense_column();
    for (size_t i = 0; i < NC; ++i) {
        auto stuff = owrk->fetch(i);
        EXPECT_EQ(stuff, rwrk->fetch(i));
    }
}

TEST_F(LoadMatrixIndexTest, TempIndex16) {
    initialize(2001, 1000, 200); // automatically use 16-bit indices.

    byteme::RawBufferWriter writer;
    write_coordinate(writer);
    const auto& buffer = writer.output;

    auto out = tatami_mtx::load_matrix_from_text_buffer<true, double, int>(buffer.data(), buffer.size());
    EXPECT_TRUE(out->prefer_rows());
    EXPECT_TRUE(out->sparse());

    auto owrk = out->dense_column();
    auto rwrk = ref->dense_column();
    for (size_t i = 0; i < NC; ++i) {
        auto stuff = owrk->fetch(i);
        EXPECT_EQ(stuff, rwrk->fetch(i));
    }
}

TEST_F(LoadMatrixIndexTest, TempIndex32) {
    initialize(2002, 100000, 2); // automatically use 32-bit indices.

    byteme::RawBufferWriter writer;
    write_coordinate(writer);
    const auto& buffer = writer.output;

    auto out = tatami_mtx::load_matrix_from_text_buffer<true, double, int>(buffer.data(), buffer.size());
    EXPECT_TRUE(out->prefer_rows());
    EXPECT_TRUE(out->sparse());

    auto owrk = out->dense_column();
    auto rwrk = ref->dense_column();
    for (size_t i = 0; i < NC; ++i) {
        auto stuff = owrk->fetch(i);
        EXPECT_EQ(stuff, rwrk->fetch(i));
    }
}

/******************************************
 *** Checking integer data type choices ***
 ******************************************/

class LoadMatrixIntegerTypeTest : public LoadMatrixTestMethods<int>, public ::testing::Test {};

TEST_F(LoadMatrixIntegerTypeTest, CoordinateAutomatic) {
    initialize(10001, 100, 200); 

    byteme::RawBufferWriter writer;
    write_coordinate(writer);
    const auto& buffer = writer.output;

    auto out = tatami_mtx::load_matrix_from_text_buffer<true, double, int>(buffer.data(), buffer.size());
    EXPECT_TRUE(out->prefer_rows());
    EXPECT_TRUE(out->sparse());

    auto owrk = out->dense_column();
    auto rwrk = ref->dense_column();
    for (size_t i = 0; i < NC; ++i) {
        auto stuff = owrk->fetch(i);
        EXPECT_EQ(stuff, rwrk->fetch(i));
    }
}

TEST_F(LoadMatrixIntegerTypeTest, CoordinateCustom) {
    initialize(10002, 100, 200); 

    byteme::RawBufferWriter writer;
    write_coordinate(writer);
    const auto& buffer = writer.output;

    auto out = tatami_mtx::load_matrix_from_text_buffer<false, double, int, int32_t>(buffer.data(), buffer.size());
    EXPECT_FALSE(out->prefer_rows());
    EXPECT_TRUE(out->sparse());

    auto owrk = out->dense_column();
    auto rwrk = ref->dense_column();
    for (size_t i = 0; i < NC; ++i) {
        auto stuff = owrk->fetch(i);
        EXPECT_EQ(stuff, rwrk->fetch(i));
    }
}

TEST_F(LoadMatrixIntegerTypeTest, ArrayAutomatic) {
    initialize(10003, 100, 200); 

    byteme::RawBufferWriter writer;
    write_array(writer);
    const auto& buffer = writer.output;

    auto out = tatami_mtx::load_matrix_from_text_buffer<true, double, int>(buffer.data(), buffer.size());
    EXPECT_TRUE(out->prefer_rows());
    EXPECT_FALSE(out->sparse());

    auto owrk = out->dense_column();
    auto rwrk = ref->dense_column();
    for (size_t i = 0; i < NC; ++i) {
        auto stuff = owrk->fetch(i);
        EXPECT_EQ(stuff, rwrk->fetch(i));
    }
}

TEST_F(LoadMatrixIntegerTypeTest, ArrayCustom) {
    initialize(10004, 100, 200); 

    byteme::RawBufferWriter writer;
    write_array(writer);
    const auto& buffer = writer.output;

    auto out = tatami_mtx::load_matrix_from_text_buffer<false, double, int, int32_t>(buffer.data(), buffer.size());
    EXPECT_FALSE(out->prefer_rows());
    EXPECT_FALSE(out->sparse());

    auto owrk = out->dense_column();
    auto rwrk = ref->dense_column();
    for (size_t i = 0; i < NC; ++i) {
        auto stuff = owrk->fetch(i);
        EXPECT_EQ(stuff, rwrk->fetch(i));
    }
}

/****************************************
 *** Checking float data type choices ***
 ****************************************/

class LoadMatrixFloatTypeTest : public LoadMatrixTestMethods<double>, public ::testing::Test {};

TEST_F(LoadMatrixFloatTypeTest, CoordinateAutomatic) {
    initialize(20001, 100, 200); 

    byteme::RawBufferWriter writer;
    write_coordinate(writer);
    const auto& buffer = writer.output;

    auto out = tatami_mtx::load_matrix_from_text_buffer<true, double, int>(buffer.data(), buffer.size());
    EXPECT_TRUE(out->prefer_rows());
    EXPECT_TRUE(out->sparse());

    auto owrk = out->dense_column();
    auto rwrk = ref->dense_column();
    for (size_t i = 0; i < NC; ++i) {
        auto stuff = owrk->fetch(i);
        EXPECT_EQ(stuff, rwrk->fetch(i));
    }
}

TEST_F(LoadMatrixFloatTypeTest, CoordinateCustom) {
    initialize(20002, 100, 200); 

    byteme::RawBufferWriter writer;
    write_coordinate(writer);
    const auto& buffer = writer.output;

    auto out = tatami_mtx::load_matrix_from_text_buffer<false, double, int, double>(buffer.data(), buffer.size());
    EXPECT_FALSE(out->prefer_rows());
    EXPECT_TRUE(out->sparse());

    auto owrk = out->dense_column();
    auto rwrk = ref->dense_column();
    for (size_t i = 0; i < NC; ++i) {
        auto stuff = owrk->fetch(i);
        EXPECT_EQ(stuff, rwrk->fetch(i));
    }
}

TEST_F(LoadMatrixFloatTypeTest, ArrayAutomatic) {
    initialize(20003, 100, 200); 

    byteme::RawBufferWriter writer;
    write_array(writer);
    const auto& buffer = writer.output;

    auto out = tatami_mtx::load_matrix_from_text_buffer<true, double, int>(buffer.data(), buffer.size());
    EXPECT_TRUE(out->prefer_rows());
    EXPECT_FALSE(out->sparse());

    auto owrk = out->dense_column();
    auto rwrk = ref->dense_column();
    for (size_t i = 0; i < NC; ++i) {
        auto stuff = owrk->fetch(i);
        EXPECT_EQ(stuff, rwrk->fetch(i));
    }
}

TEST_F(LoadMatrixFloatTypeTest, ArrayCustom) {
    initialize(20004, 100, 200); 

    byteme::RawBufferWriter writer;
    write_array(writer);
    const auto& buffer = writer.output;

    auto out = tatami_mtx::load_matrix_from_text_buffer<false, double, int, double>(buffer.data(), buffer.size());
    EXPECT_FALSE(out->prefer_rows());
    EXPECT_FALSE(out->sparse());

    auto owrk = out->dense_column();
    auto rwrk = ref->dense_column();
    for (size_t i = 0; i < NC; ++i) {
        auto stuff = owrk->fetch(i);
        EXPECT_EQ(stuff, rwrk->fetch(i));
    }
}
