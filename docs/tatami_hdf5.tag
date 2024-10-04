<?xml version='1.0' encoding='UTF-8' standalone='yes' ?>
<tagfile doxygen_version="1.12.0">
  <compound kind="file">
    <name>load_matrix.hpp</name>
    <path>tatami_mtx/</path>
    <filename>load__matrix_8hpp.html</filename>
    <class kind="struct">tatami_mtx::Automatic</class>
    <class kind="struct">tatami_mtx::Options</class>
    <namespace>tatami_mtx</namespace>
  </compound>
  <compound kind="file">
    <name>tatami_mtx.hpp</name>
    <path>tatami_mtx/</path>
    <filename>tatami__mtx_8hpp.html</filename>
    <namespace>tatami_mtx</namespace>
  </compound>
  <compound kind="struct">
    <name>tatami_mtx::Automatic</name>
    <filename>structtatami__mtx_1_1Automatic.html</filename>
  </compound>
  <compound kind="struct">
    <name>tatami_mtx::Options</name>
    <filename>structtatami__mtx_1_1Options.html</filename>
    <member kind="variable">
      <type>bool</type>
      <name>row</name>
      <anchorfile>structtatami__mtx_1_1Options.html</anchorfile>
      <anchor>ab8c5093ae706e577282bf477fe72c737</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>size_t</type>
      <name>buffer_size</name>
      <anchorfile>structtatami__mtx_1_1Options.html</anchorfile>
      <anchor>a14902272ff0a08ce271f11fa420355f0</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>bool</type>
      <name>parallel</name>
      <anchorfile>structtatami__mtx_1_1Options.html</anchorfile>
      <anchor>abe6fc86b8b6b6c4c09f39bd3637d41cf</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>compression</name>
      <anchorfile>structtatami__mtx_1_1Options.html</anchorfile>
      <anchor>aa9832489c1d2d8a697f5d822219e4738</anchor>
      <arglist></arglist>
    </member>
  </compound>
  <compound kind="namespace">
    <name>tatami_mtx</name>
    <filename>namespacetatami__mtx.html</filename>
    <class kind="struct">tatami_mtx::Automatic</class>
    <class kind="struct">tatami_mtx::Options</class>
    <member kind="function">
      <type>std::shared_ptr&lt; tatami::Matrix&lt; Value_, Index_ &gt; &gt;</type>
      <name>load_matrix</name>
      <anchorfile>namespacetatami__mtx.html</anchorfile>
      <anchor>a0ae14375d217b2c9a049317aa8209f05</anchor>
      <arglist>(byteme::Reader &amp;reader, const Options &amp;options)</arglist>
    </member>
    <member kind="function">
      <type>std::shared_ptr&lt; tatami::Matrix&lt; Value_, Index_ &gt; &gt;</type>
      <name>load_matrix_from_text_file</name>
      <anchorfile>namespacetatami__mtx.html</anchorfile>
      <anchor>af7f6bd31cf0ba74bb5a2d14180b66b16</anchor>
      <arglist>(const char *filepath, const Options &amp;options)</arglist>
    </member>
    <member kind="function">
      <type>std::shared_ptr&lt; tatami::Matrix&lt; Value_, Index_ &gt; &gt;</type>
      <name>load_matrix_from_gzip_file</name>
      <anchorfile>namespacetatami__mtx.html</anchorfile>
      <anchor>ae9da28ce51cdff3c12e23d353663fc85</anchor>
      <arglist>(const char *filepath, const Options &amp;options)</arglist>
    </member>
    <member kind="function">
      <type>std::shared_ptr&lt; tatami::Matrix&lt; Value_, Index_ &gt; &gt;</type>
      <name>load_matrix_from_some_file</name>
      <anchorfile>namespacetatami__mtx.html</anchorfile>
      <anchor>a45af5d93e47b2c3c90582649fa4364ae</anchor>
      <arglist>(const char *filepath, const Options &amp;options)</arglist>
    </member>
    <member kind="function">
      <type>std::shared_ptr&lt; tatami::Matrix&lt; Value_, Index_ &gt; &gt;</type>
      <name>load_matrix_from_text_buffer</name>
      <anchorfile>namespacetatami__mtx.html</anchorfile>
      <anchor>a6af1bcb6033a43f7bd9fab5a3bc5b469</anchor>
      <arglist>(const unsigned char *buffer, size_t n, const Options &amp;options)</arglist>
    </member>
    <member kind="function">
      <type>std::shared_ptr&lt; tatami::Matrix&lt; Value_, Index_ &gt; &gt;</type>
      <name>load_matrix_from_zlib_buffer</name>
      <anchorfile>namespacetatami__mtx.html</anchorfile>
      <anchor>adbb55dd4fc75380091bbb27447b2fede</anchor>
      <arglist>(const unsigned char *buffer, size_t n, const Options &amp;options)</arglist>
    </member>
    <member kind="function">
      <type>std::shared_ptr&lt; tatami::Matrix&lt; Value_, Index_ &gt; &gt;</type>
      <name>load_matrix_from_some_buffer</name>
      <anchorfile>namespacetatami__mtx.html</anchorfile>
      <anchor>a433952bcfcf1b74a8a90c2257ec54b4c</anchor>
      <arglist>(const unsigned char *buffer, size_t n, const Options &amp;options)</arglist>
    </member>
  </compound>
  <compound kind="page">
    <name>index</name>
    <title>Create &lt;tt&gt;tatami&lt;/tt&gt; matrices from Matrix Market files</title>
    <filename>index.html</filename>
    <docanchor file="index.html">md__2github_2workspace_2README</docanchor>
  </compound>
</tagfile>
