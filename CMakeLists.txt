# CMakeList.txt : Socket の CMake プロジェクト。ソースを含めて、次を定義します:
# プロジェクト専用ロジックはこちらです。
#
cmake_minimum_required (VERSION 3.10)

# サポートされている場合は、MSVC コンパイラのホット リロードを有効にします。
if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

project ("Socket")

# ソースをこのプロジェクトの実行可能ファイルに追加します。
add_executable (Socket
	"Socket.cpp"

	# include
	"include/common.h"
	"include/Packet.h"
	"include/Socket.h"

	# include/Cryptgraphy
	"include/Cryptgraphy/AES128.h"
	"include/Cryptgraphy/common.h"
	"include/Cryptgraphy/ECDSA.h"
	"include/Cryptgraphy/ECPoint.h"
	"include/Cryptgraphy/KeyManager.h"
	"include/Cryptgraphy/ModInt.h"
	"include/Cryptgraphy/MultiWordInt.h"
	"include/Cryptgraphy/NumberSet.h"
	"include/Cryptgraphy/RandomGenerator.h"
	"include/Cryptgraphy/SHAKE256.h"
)

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET Socket PROPERTY CXX_STANDARD 20)
endif()

# TODO: テストを追加し、必要な場合は、ターゲットをインストールします。
