<?xml version='1.0' encoding='UTF-8' standalone='yes' ?>
<tagfile doxygen_version="1.12.0">
  <compound kind="file">
    <name>load_matrix.hpp</name>
    <path>tatami_mtx/</path>
    <filename>load__matrix_8hpp.html</filename>
    <class kind="struct">tatami_mtx::Automatic</class>
    <class kind="struct">tatami_mtx::LoadMatrixOptions</class>
    <namespace>tatami_mtx</namespace>
  </compound>
  <compound kind="file">
    <name>tatami_mtx.hpp</name>
    <path>tatami_mtx/</path>
    <filename>tatami__mtx_8hpp.html</filename>
    <namespace>tatami_mtx</namespace>
  </compound>
  <compound kind="file">
    <name>write_matrix.hpp</name>
    <path>tatami_mtx/</path>
    <filename>write__matrix_8hpp.html</filename>
    <class kind="struct">tatami_mtx::WriteMatrixOptions</class>
    <namespace>tatami_mtx</namespace>
  </compound>
  <compound kind="struct">
    <name>tatami_mtx::Automatic</name>
    <filename>structtatami__mtx_1_1Automatic.html</filename>
  </compound>
  <compound kind="struct">
    <name>tatami_mtx::LoadMatrixOptions</name>
    <filename>structtatami__mtx_1_1LoadMatrixOptions.html</filename>
    <member kind="variable">
      <type>bool</type>
      <name>row</name>
      <anchorfile>structtatami__mtx_1_1LoadMatrixOptions.html</anchorfile>
      <anchor>af152ee1d02c76bf45fc792d7749ab7cd</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>std::size_t</type>
      <name>buffer_size</name>
      <anchorfile>structtatami__mtx_1_1LoadMatrixOptions.html</anchorfile>
      <anchor>a40e8e6b35775ff806897d1165d49fbcb</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>num_threads</name>
      <anchorfile>structtatami__mtx_1_1LoadMatrixOptions.html</anchorfile>
      <anchor>a0633b427f4a4a2ba642bf4306d0496bc</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>compression</name>
      <anchorfile>structtatami__mtx_1_1LoadMatrixOptions.html</anchorfile>
      <anchor>a875ab24a34c434ae6472aa70cbe5e449</anchor>
      <arglist></arglist>
    </member>
  </compound>
  <compound kind="struct">
    <name>tatami_mtx::WriteMatrixOptions</name>
    <filename>structtatami__mtx_1_1WriteMatrixOptions.html</filename>
    <member kind="variable">
      <type>std::optional&lt; bool &gt;</type>
      <name>coordinate</name>
      <anchorfile>structtatami__mtx_1_1WriteMatrixOptions.html</anchorfile>
      <anchor>a6f14b513ae476cf9b6a278f1cb2b69aa</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>std::optional&lt; bool &gt;</type>
      <name>by_row</name>
      <anchorfile>structtatami__mtx_1_1WriteMatrixOptions.html</anchorfile>
      <anchor>af8f8feddc7bb2d782bffba7f49791d56</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>bool</type>
      <name>banner</name>
      <anchorfile>structtatami__mtx_1_1WriteMatrixOptions.html</anchorfile>
      <anchor>ae08327733442645f9a834056adc3535e</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>std::optional&lt; std::chars_format &gt;</type>
      <name>format</name>
      <anchorfile>structtatami__mtx_1_1WriteMatrixOptions.html</anchorfile>
      <anchor>a02d59ae99916f161fa1b0a5843b5c555</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>std::optional&lt; int &gt;</type>
      <name>precision</name>
      <anchorfile>structtatami__mtx_1_1WriteMatrixOptions.html</anchorfile>
      <anchor>a3df0a3d85827f1434d7a5c6ebb874d7b</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>num_threads</name>
      <anchorfile>structtatami__mtx_1_1WriteMatrixOptions.html</anchorfile>
      <anchor>a25b45b67de6fdfa897d72afe7179fed6</anchor>
      <arglist></arglist>
    </member>
  </compound>
  <compound kind="namespace">
    <name>tatami_mtx</name>
    <filename>namespacetatami__mtx.html</filename>
    <class kind="struct">tatami_mtx::Automatic</class>
    <class kind="struct">tatami_mtx::LoadMatrixOptions</class>
    <class kind="struct">tatami_mtx::WriteMatrixOptions</class>
    <member kind="function">
      <type>std::shared_ptr&lt; tatami::Matrix&lt; Value_, Index_ &gt; &gt;</type>
      <name>load_matrix</name>
      <anchorfile>namespacetatami__mtx.html</anchorfile>
      <anchor>a4e16806a54f17fd96c2c404dbdba88ce</anchor>
      <arglist>(byteme::Reader &amp;reader, const LoadMatrixOptions &amp;options)</arglist>
    </member>
    <member kind="function">
      <type>std::shared_ptr&lt; tatami::Matrix&lt; Value_, Index_ &gt; &gt;</type>
      <name>load_matrix_from_text_file</name>
      <anchorfile>namespacetatami__mtx.html</anchorfile>
      <anchor>ad30a74ea0b689749b0bfe1d346038792</anchor>
      <arglist>(const char *filepath, const LoadMatrixOptions &amp;options)</arglist>
    </member>
    <member kind="function">
      <type>std::shared_ptr&lt; tatami::Matrix&lt; Value_, Index_ &gt; &gt;</type>
      <name>load_matrix_from_gzip_file</name>
      <anchorfile>namespacetatami__mtx.html</anchorfile>
      <anchor>acd682d0c2b376eadbe3e54c5f3adee95</anchor>
      <arglist>(const char *filepath, const LoadMatrixOptions &amp;options)</arglist>
    </member>
    <member kind="function">
      <type>std::shared_ptr&lt; tatami::Matrix&lt; Value_, Index_ &gt; &gt;</type>
      <name>load_matrix_from_some_file</name>
      <anchorfile>namespacetatami__mtx.html</anchorfile>
      <anchor>acf204b49245947b19d1b842743bf9329</anchor>
      <arglist>(const char *filepath, const LoadMatrixOptions &amp;options)</arglist>
    </member>
    <member kind="function">
      <type>std::shared_ptr&lt; tatami::Matrix&lt; Value_, Index_ &gt; &gt;</type>
      <name>load_matrix_from_text_buffer</name>
      <anchorfile>namespacetatami__mtx.html</anchorfile>
      <anchor>a81126d06ea0867eeab913b546adb332b</anchor>
      <arglist>(const unsigned char *buffer, const std::size_t n, const LoadMatrixOptions &amp;options)</arglist>
    </member>
    <member kind="function">
      <type>std::shared_ptr&lt; tatami::Matrix&lt; Value_, Index_ &gt; &gt;</type>
      <name>load_matrix_from_zlib_buffer</name>
      <anchorfile>namespacetatami__mtx.html</anchorfile>
      <anchor>a1079725e7a2591883a99af6e46d745ae</anchor>
      <arglist>(const unsigned char *buffer, const std::size_t n, const LoadMatrixOptions &amp;options)</arglist>
    </member>
    <member kind="function">
      <type>std::shared_ptr&lt; tatami::Matrix&lt; Value_, Index_ &gt; &gt;</type>
      <name>load_matrix_from_some_buffer</name>
      <anchorfile>namespacetatami__mtx.html</anchorfile>
      <anchor>aabc483a3cc6cd0d6e25f3aa43e5ed2af</anchor>
      <arglist>(const unsigned char *buffer, const std::size_t n, const LoadMatrixOptions &amp;options)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>write_matrix</name>
      <anchorfile>namespacetatami__mtx.html</anchorfile>
      <anchor>aba80c675ff72592b7fd7a96533c540bb</anchor>
      <arglist>(const tatami::Matrix&lt; Value_, Index_ &gt; &amp;matrix, byteme::Writer &amp;writer, const WriteMatrixOptions &amp;options)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>write_matrix_to_text_file</name>
      <anchorfile>namespacetatami__mtx.html</anchorfile>
      <anchor>a71f5b96f2d1778027f1f6372f8f5a3ce</anchor>
      <arglist>(const tatami::Matrix&lt; Value_, Index_ &gt; &amp;matrix, const char *filepath, const WriteMatrixOptions &amp;options)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>write_matrix_to_gzip_file</name>
      <anchorfile>namespacetatami__mtx.html</anchorfile>
      <anchor>a3e0ecb3c298e62b2996289656d35d26e</anchor>
      <arglist>(const tatami::Matrix&lt; Value_, Index_ &gt; &amp;matrix, const char *filepath, const WriteMatrixOptions &amp;options)</arglist>
    </member>
    <member kind="function">
      <type>std::vector&lt; unsigned char &gt;</type>
      <name>write_matrix_to_text_buffer</name>
      <anchorfile>namespacetatami__mtx.html</anchorfile>
      <anchor>a3a142b96c4c04fecae361e5e500f3ce6</anchor>
      <arglist>(const tatami::Matrix&lt; Value_, Index_ &gt; &amp;matrix, const WriteMatrixOptions &amp;options)</arglist>
    </member>
    <member kind="function">
      <type>std::vector&lt; unsigned char &gt;</type>
      <name>write_matrix_to_zlib_buffer</name>
      <anchorfile>namespacetatami__mtx.html</anchorfile>
      <anchor>a5a89ea59a47c8bd4c3283bf75ae6a4bd</anchor>
      <arglist>(const tatami::Matrix&lt; Value_, Index_ &gt; &amp;matrix, const int mode, const WriteMatrixOptions &amp;options)</arglist>
    </member>
  </compound>
  <compound kind="page">
    <name>index</name>
    <title>Create &lt;tt&gt;tatami&lt;/tt&gt; matrices from Matrix Market files</title>
    <filename>index.html</filename>
    <docanchor file="index.html">md__2github_2workspace_2README</docanchor>
  </compound>
</tagfile>
