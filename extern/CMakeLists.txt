include(FetchContent)

FetchContent_Declare(
  tatami
  GIT_REPOSITORY https://github.com/tatami-inc/tatami
  GIT_TAG 53c95665dc965a9827d84f65238632425069d920
)

FetchContent_MakeAvailable(tatami)

FetchContent_Declare(
  eminem
  GIT_REPOSITORY https://github.com/tatami-inc/eminem
  GIT_TAG fc6ee36615e1d5f4a597d6ff5622960a10bc9ed7
)

FetchContent_MakeAvailable(eminem)
