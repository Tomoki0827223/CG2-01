#include "SrvManager.h"
#include "DirectXCommon.h" 
#include <cassert> 

// グローバル定数の定義 (SrvManager::kMaxSrvCount)
const uint32_t SrvManager::kMaxSrvCount;

void SrvManager::Initialize(DirectXCommon* dxCommon) {
    directXCommon_ = dxCommon;

    // SRVヒープの生成 (4引数: Device, Type, NumDescriptors, ShaderVisible)
    descriptorHeap_ = directXCommon_->CreateDescriptorHeap(
        directXCommon_->GetDevice(),            // 1. Device
        D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, // 2. Type
        kMaxSrvCount,                           // 3. NumDescriptors
        true                                    // 4. ShaderVisible
    );

    // ディスクリプタ1個分のサイズを取得
    descriptorSize_ = directXCommon_->GetDevice()->GetDescriptorHandleIncrementSize(
        D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV
    );

    // ImGuiが0番を使用するため、1番から利用を開始
    useIndex_ = 1;
}


// SRVインデックスからCPUハンドルを計算
D3D12_CPU_DESCRIPTOR_HANDLE SrvManager::GetCPUDescriptorHandle(uint32_t index) const {
    assert(index < kMaxSrvCount);
    // ヒープの先頭アドレスを取得
    D3D12_CPU_DESCRIPTOR_HANDLE handleCPU = descriptorHeap_->GetCPUDescriptorHandleForHeapStart();
    // オフセット計算: 先頭 + (インデックス * サイズ)
    handleCPU.ptr += (size_t)descriptorSize_ * index;
    return handleCPU;
}

// SRVインデックスからGPUハンドルを計算
D3D12_GPU_DESCRIPTOR_HANDLE SrvManager::GetGPUDescriptorHandle(uint32_t index) const {
    assert(index < kMaxSrvCount);
    // ヒープの先頭アドレスを取得
    D3D12_GPU_DESCRIPTOR_HANDLE handleGPU = descriptorHeap_->GetGPUDescriptorHandleForHeapStart();
    // オフセット計算: 先頭 + (インデックス * サイズ)
    handleGPU.ptr += (size_t)descriptorSize_ * index;
    return handleGPU;
}

// SRVインデックスの確保
uint32_t SrvManager::Allocate() {
    assert(useIndex_ < kMaxSrvCount); // 最大数を超えていないかチェック

    uint32_t index = useIndex_;
    useIndex_++;
    return index;
}

// 確保可能チェック
bool SrvManager::CanAllocate() const {
    return useIndex_ < kMaxSrvCount;
}

// 描画前処理
void SrvManager::PreDraw() {
    // SRVヒープをコマンドリストにセット
    ID3D12DescriptorHeap* descriptorHeaps[] = { descriptorHeap_.Get() };
    directXCommon_->GetCommandList()->SetDescriptorHeaps(_countof(descriptorHeaps), descriptorHeaps);
}