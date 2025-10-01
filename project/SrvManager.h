#pragma once

#include <cstdint>
#include <wrl.h>
#include <d3d12.h>
#include <vector>

class DirectXCommon; // 前方宣言

class SrvManager {
private:
    // 最大SRV数 (テクスチャ、Structured Bufferなどで共有)
    static const uint32_t kMaxSrvCount = 512;

    // SRV用ディスクリプタヒープ
    Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> descriptorHeap_;

    // SRV用ディスクリプタサイズ (1個分のサイズ)
    uint32_t descriptorSize_ = 0;

    // 次に使用するSRVインデックス
    uint32_t useIndex_ = 0;

    // DirectXCommonのポインタ
    DirectXCommon* directXCommon_ = nullptr;

public:
    // 初期化 (DirectXCommonのポインタを受け取り、ヒープを生成)
    void Initialize(DirectXCommon* dxCommon);

    // デスクリプタハンドル計算関数
    D3D12_CPU_DESCRIPTOR_HANDLE GetCPUDescriptorHandle(uint32_t index) const;
    D3D12_GPU_DESCRIPTOR_HANDLE GetGPUDescriptorHandle(uint32_t index) const;

    // 確保関数 (SRVインデックスを取得)
    uint32_t Allocate();

    // 確保可能チェック
    bool CanAllocate() const;

    // 描画前処理 (SRVヒープをコマンドリストにセット)
    void PreDraw();
};