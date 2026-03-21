#include "tatami_test/tatami_test.hpp"
#include "eminem/eminem.hpp"

#include "tatami_mtx/load_matrix.hpp"
#include "tatami_mtx/write_matrix.hpp"
#include "temp_file_path.h"

#include <string>
#include <vector>

TEST(WriteConvert, Integer) {
    std::vector<char> buffer(1);

    {
        auto used = tatami_mtx::convert(10, buffer, {}, {});
        EXPECT_EQ(used, 2);
        auto bufptr = reinterpret_cast<const char*>(buffer.data());
        EXPECT_EQ("10", std::string(bufptr, bufptr + used));
    }

    // Re-use is fine.
    {
        auto used = tatami_mtx::convert(-5, buffer, {}, {});
        EXPECT_EQ(used, 2);
        auto bufptr = reinterpret_cast<const char*>(buffer.data());
        EXPECT_EQ("-5", std::string(bufptr, bufptr + used));
    }

    {
        auto used = tatami_mtx::convert(123, buffer, {}, {});
        EXPECT_EQ(used, 3);
        auto bufptr = reinterpret_cast<const char*>(buffer.data());
        EXPECT_EQ("123", std::string(bufptr, bufptr + used));
    }
}

TEST(WriteConvert, Float) {
    std::vector<char> buffer(1);

    {
        auto used = tatami_mtx::convert(10.0, buffer, {}, {});
        EXPECT_EQ(used, 2);
        auto bufptr = reinterpret_cast<const char*>(buffer.data());
        EXPECT_EQ("10", std::string(bufptr, bufptr + used));
    }

    {
        auto used = tatami_mtx::convert(15.0, buffer, std::chars_format::scientific, {});
        EXPECT_GT(used, 2);
        auto bufptr = reinterpret_cast<const char*>(buffer.data());
        std::string converted(bufptr, bufptr + used);
        EXPECT_TRUE(converted.find("1.5") != std::string::npos);
        EXPECT_TRUE(
            converted.find("e") != std::string::npos ||
            converted.find("E") != std::string::npos
        );
    }

    {
        auto used = tatami_mtx::convert(-1234.567890, buffer, std::chars_format::scientific, 5);
        EXPECT_GT(used, 2);
        auto bufptr = reinterpret_cast<const char*>(buffer.data());
        std::string converted(bufptr, bufptr + used);
        EXPECT_TRUE(converted.find("-1.2345") != std::string::npos);
        EXPECT_TRUE(
            converted.find("e") != std::string::npos ||
            converted.find("E") != std::string::npos
        );
    }
}

TEST(WriteMatrix, DenseArray) {
    const int NR = 40, NC = 25;
    auto vec = tatami_test::simulate_vector<double>(NR * NC, {});
    tatami::DenseMatrix<double, int, std::vector<double> > mat(NR, NC, vec, true);
    auto path = temp_file_path("tatami_mtx-test-write_matrix");
    tatami_mtx::write_matrix_to_text_file(mat, path.c_str(), {});

    { 
        byteme::RawFileReader reader(path.c_str(), {});
        eminem::Parser<decltype(&reader), int> parser(&reader, {});

        parser.scan_preamble();
        const auto& banner = parser.get_banner();
        EXPECT_EQ(banner.field, eminem::Field::REAL);
        EXPECT_EQ(banner.format, eminem::Format::ARRAY);
    }

    auto reloaded = tatami_mtx::load_matrix_from_text_file<double, int>(path.c_str(), {});
    EXPECT_FALSE(reloaded->is_sparse());
    EXPECT_EQ(reloaded->nrow(), NR);
    EXPECT_EQ(reloaded->ncol(), NC);

    auto ext = reloaded->dense_row();
    std::vector<double> buffer(NC);
    for (int r = 0; r < NR; ++r) {
        auto ptr = ext->fetch(r, buffer.data());
        for (int c = 0; c < NC; ++c) {
            EXPECT_FLOAT_EQ(ptr[c], vec[sanisizer::nd_offset<std::size_t>(c, NC, r)]);
        }
    }
}

class WriteMatrixCoordinateTest : public ::testing::TestWithParam<int> {};

TEST_P(WriteMatrixCoordinateTest, Dense) {
    const int NR = 40, NC = 25;
    auto vec = tatami_test::simulate_vector<double>(NR * NC, {});
    tatami::DenseMatrix<double, int, std::vector<double> > mat(NR, NC, vec, true);

    const auto scenario = GetParam(); // 0 = automatic, 1 = by column, 2 = by row.
    auto path = temp_file_path("tatami_mtx-test-write_matrix");
    tatami_mtx::write_matrix_to_text_file(mat, path.c_str(), [&](){
        tatami_mtx::WriteMatrixOptions opt;
        opt.coordinate = true;
        if (scenario) {
            opt.by_row = (scenario == 2);
        }
        return opt;
    }());

    { 
        byteme::RawFileReader reader(path.c_str(), {});
        eminem::Parser<decltype(&reader), int> parser(&reader, {});

        parser.scan_preamble();
        const auto& banner = parser.get_banner();
        EXPECT_EQ(banner.field, eminem::Field::REAL);
        EXPECT_EQ(banner.format, eminem::Format::COORDINATE);
    }

    auto reloaded = tatami_mtx::load_matrix_from_text_file<double, int>(path.c_str(), {});
    EXPECT_TRUE(reloaded->is_sparse());
    EXPECT_EQ(reloaded->nrow(), NR);
    EXPECT_EQ(reloaded->ncol(), NC);

    auto ext = reloaded->dense_row();
    std::vector<double> buffer(NC);
    for (int r = 0; r < NR; ++r) {
        auto ptr = ext->fetch(r, buffer.data());
        for (int c = 0; c < NC; ++c) {
            EXPECT_FLOAT_EQ(ptr[c], vec[sanisizer::nd_offset<std::size_t>(c, NC, r)]);
        }
    }
}

TEST_P(WriteMatrixCoordinateTest, Sparse) {
    const int NR = 400, NC = 250;
    auto sim = tatami_test::simulate_compressed_sparse<double>(NC, NR, {});
    tatami::CompressedSparseMatrix<
        double,
        int,
        std::vector<double>,
        std::vector<int>,
        std::vector<std::size_t>
    > mat(NR, NC, std::move(sim.data), std::move(sim.index), std::move(sim.indptr), false);

    const auto scenario = GetParam(); // 0 = automatic, 1 = by column, 2 = by row.
    auto path = temp_file_path("tatami_mtx-test-write_matrix");
    tatami_mtx::write_matrix_to_text_file(mat, path.c_str(), [&](){
        tatami_mtx::WriteMatrixOptions opt;
        if (scenario) {
            opt.by_row = (scenario == 2);
        }
        return opt;
    }());

    { 
        byteme::RawFileReader reader(path.c_str(), {});
        eminem::Parser<decltype(&reader), int> parser(&reader, {});

        parser.scan_preamble();
        const auto& banner = parser.get_banner();
        EXPECT_EQ(banner.field, eminem::Field::REAL);
        EXPECT_EQ(banner.format, eminem::Format::COORDINATE);
    }

    auto reloaded = tatami_mtx::load_matrix_from_text_file<double, int>(path.c_str(), {});
    EXPECT_TRUE(reloaded->is_sparse());
    EXPECT_EQ(reloaded->nrow(), NR);
    EXPECT_EQ(reloaded->ncol(), NC);

    auto ext = reloaded->dense_row();
    auto refext = mat.dense_row();
    std::vector<double> buffer(NC);
    std::vector<double> refbuffer(NC);
    for (int r = 0; r < NR; ++r) {
        auto ptr = ext->fetch(r, buffer.data());
        auto refptr = refext ->fetch(r, refbuffer.data());
        for (int c = 0; c < NC; ++c) {
            EXPECT_FLOAT_EQ(ptr[c], refptr[c]);
        }
    }
}

INSTANTIATE_TEST_SUITE_P(
    WriteMatrix,
    WriteMatrixCoordinateTest,
    ::testing::Values(0, 1, 2) // 0 = automatic, 1 = by column, 2 = by row.
);

TEST(WriteMatrix, SparseParallel) {
    const int NR = 400, NC = 250;

    for (int i = 0; i < 2; ++i) {
        auto sim = tatami_test::simulate_compressed_sparse<double>(
            (i == 0 ? NR : NC),
            (i == 0 ? NC : NR),
            {}
        );

        tatami::CompressedSparseMatrix<
            double,
            int,
            std::vector<double>,
            std::vector<int>,
            std::vector<std::size_t>
        > mat(NR, NC, std::move(sim.data), std::move(sim.index), std::move(sim.indptr), /* by_row = */ i == 0);

        auto path = temp_file_path("tatami_mtx-test-write_matrix-parallel");
        tatami_mtx::write_matrix_to_text_file(mat, path.c_str(), [&](){
            tatami_mtx::WriteMatrixOptions opt;
            opt.num_threads = 3;
            return opt;
        }());

        auto reloaded = tatami_mtx::load_matrix_from_text_file<double, int>(path.c_str(), {});
        EXPECT_TRUE(reloaded->is_sparse());
        EXPECT_EQ(reloaded->nrow(), NR);
        EXPECT_EQ(reloaded->ncol(), NC);

        auto ext = reloaded->dense_row();
        auto refext = mat.dense_row();
        std::vector<double> buffer(NC);
        std::vector<double> refbuffer(NC);
        for (int r = 0; r < NR; ++r) {
            auto ptr = ext->fetch(r, buffer.data());
            auto refptr = refext ->fetch(r, refbuffer.data());
            for (int c = 0; c < NC; ++c) {
                EXPECT_FLOAT_EQ(ptr[c], refptr[c]);
            }
        }
    }
}

TEST(WriteMatrix, EmptyCoordinate) {
    for (int i = 0; i < 2; ++i) {
        const auto current_nrow = (i == 0 ? 10 : 0);
        const auto current_ncol = (i == 0 ? 0 : 20);

        tatami::CompressedSparseMatrix<
            double,
            int,
            std::vector<double>,
            std::vector<int>,
            std::vector<std::size_t>
        > mat(
            current_nrow,
            current_ncol,
            std::vector<double>(),
            std::vector<int>(),
            std::vector<std::size_t>(current_ncol + 1),
            false
        );

        auto path = temp_file_path("tatami_mtx-test-write_matrix");
        tatami_mtx::write_matrix_to_text_file(mat, path.c_str(), [&](){
            tatami_mtx::WriteMatrixOptions opt;
            opt.coordinate = true;
            if (i) {
                opt.by_row = (i == 2);
            }
            return opt;
        }());

        auto reloaded = tatami_mtx::load_matrix_from_text_file<double, int>(path.c_str(), {});
        EXPECT_EQ(reloaded->nrow(), current_nrow);
        EXPECT_EQ(reloaded->ncol(), current_ncol);
    }
}

TEST(WriteMatrix, Integer) {
    const int NR = 40, NC = 25;
    auto vec = tatami_test::simulate_vector<int>(NR * NC, [&](){
        tatami_test::SimulateVectorOptions opt;
        opt.lower = -10;
        opt.upper = 10;
        return opt;
    }());

    tatami::DenseMatrix<int, std::size_t, std::vector<int> > mat(NR, NC, vec, true);
    auto path = temp_file_path("tatami_mtx-test-write_matrix");
    tatami_mtx::write_matrix_to_text_file(mat, path.c_str(), {});

    { 
        byteme::RawFileReader reader(path.c_str(), {});
        eminem::Parser<decltype(&reader), int> parser(&reader, {});

        parser.scan_preamble();
        const auto& banner = parser.get_banner();
        EXPECT_EQ(banner.field, eminem::Field::INTEGER);
        EXPECT_EQ(banner.format, eminem::Format::ARRAY);
    }

    auto reloaded = tatami_mtx::load_matrix_from_text_file<double, int>(path.c_str(), {});
    auto ext = reloaded->dense_row();
    std::vector<double> buffer(NC);
    for (int r = 0; r < NR; ++r) {
        auto ptr = ext->fetch(r, buffer.data());
        for (int c = 0; c < NC; ++c) {
            EXPECT_EQ(ptr[c], vec[sanisizer::nd_offset<std::size_t>(c, NC, r)]);
        }
    }
}

TEST(WriteMatrix, TextBuffer) {
    const int NR = 40, NC = 25;
    auto vec = tatami_test::simulate_vector<double>(NR * NC, {});
    tatami::DenseMatrix<double, int, std::vector<double> > mat(NR, NC, vec, true);

    auto buf = tatami_mtx::write_matrix_to_text_buffer(mat, {});
    auto reloaded = tatami_mtx::load_matrix_from_text_buffer<double, int>(buf.data(), buf.size(), {});
    EXPECT_FALSE(reloaded->is_sparse());
    EXPECT_EQ(reloaded->nrow(), NR);
    EXPECT_EQ(reloaded->ncol(), NC);

    auto ext = reloaded->dense_row();
    std::vector<double> buffer(NC);
    for (int r = 0; r < NR; ++r) {
        auto ptr = ext->fetch(r, buffer.data());
        for (int c = 0; c < NC; ++c) {
            EXPECT_FLOAT_EQ(ptr[c], vec[sanisizer::nd_offset<std::size_t>(c, NC, r)]);
        }
    }
}

TEST(WriteMatrix, GzipFile) {
    const int NR = 40, NC = 25;
    auto vec = tatami_test::simulate_vector<double>(NR * NC, {});
    tatami::DenseMatrix<double, int, std::vector<double> > mat(NR, NC, vec, true);

    auto path = temp_file_path("tatami_mtx-test-write_matrix");
    tatami_mtx::write_matrix_to_gzip_file(mat, path.c_str(), {});

    auto reloaded = tatami_mtx::load_matrix_from_gzip_file<double, int>(path.c_str(), {});
    EXPECT_FALSE(reloaded->is_sparse());
    EXPECT_EQ(reloaded->nrow(), NR);
    EXPECT_EQ(reloaded->ncol(), NC);

    auto ext = reloaded->dense_row();
    std::vector<double> buffer(NC);
    for (int r = 0; r < NR; ++r) {
        auto ptr = ext->fetch(r, buffer.data());
        for (int c = 0; c < NC; ++c) {
            EXPECT_FLOAT_EQ(ptr[c], vec[sanisizer::nd_offset<std::size_t>(c, NC, r)]);
        }
    }
}

TEST(WriteMatrix, ZlibBuffer) {
    const int NR = 40, NC = 25;
    auto vec = tatami_test::simulate_vector<double>(NR * NC, {});
    tatami::DenseMatrix<double, int, std::vector<double> > mat(NR, NC, vec, true);

    auto buf = tatami_mtx::write_matrix_to_zlib_buffer(mat, byteme::ZlibCompressionMode::ZLIB, {});
    auto reloaded = tatami_mtx::load_matrix_from_zlib_buffer<double, int>(buf.data(), buf.size(), {});
    EXPECT_FALSE(reloaded->is_sparse());
    EXPECT_EQ(reloaded->nrow(), NR);
    EXPECT_EQ(reloaded->ncol(), NC);

    auto ext = reloaded->dense_row();
    std::vector<double> buffer(NC);
    for (int r = 0; r < NR; ++r) {
        auto ptr = ext->fetch(r, buffer.data());
        for (int c = 0; c < NC; ++c) {
            EXPECT_FLOAT_EQ(ptr[c], vec[sanisizer::nd_offset<std::size_t>(c, NC, r)]);
        }
    }
}
