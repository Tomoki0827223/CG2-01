#include "StringUitilty.h"

namespace StringUitilty {

    std::wstring ConvertString(const std::string& str)
    {
        if (str.empty()) {
            return std::wstring();
        }

        // 必要なサイズを取得
        int sizeNeeded = MultiByteToWideChar(CP_UTF8, 0, str.data(), static_cast<int>(str.size()), nullptr, 0);
        if (sizeNeeded == 0) {
            return std::wstring(); // 変換に失敗
        }

        // 結果の wstring を作成
        std::wstring result(sizeNeeded, L'\0');
        MultiByteToWideChar(CP_UTF8, 0, str.data(), static_cast<int>(str.size()), &result[0], sizeNeeded);

        return result;
    }

    std::string ConvertString(const std::wstring& str)
    {
        if (str.empty()) {
            return std::string();
        }

        // 必要なサイズを取得
        int sizeNeeded = WideCharToMultiByte(CP_UTF8, 0, str.data(), static_cast<int>(str.size()), nullptr, 0, nullptr, nullptr);
        if (sizeNeeded == 0) {
            return std::string(); // 変換に失敗
        }

        // 結果の string を作成
        std::string result(sizeNeeded, '\0');
        WideCharToMultiByte(CP_UTF8, 0, str.data(), static_cast<int>(str.size()), &result[0], sizeNeeded, nullptr, nullptr);

        return result;
    }

}
