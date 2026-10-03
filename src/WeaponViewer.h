#pragma once
// Reusable renderer. Does not own the host's device, frame loop, or swap chain.
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <d3d11.h>
#include <d3dcompiler.h>
#include <DirectXMath.h>
#include <wrl/client.h>
#include <wincodec.h>
#include <commdlg.h>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <vector>
#include <map>
#include <set>
#include <string>
#include <cstring>
#include <cstdint>
#include <cctype>
#include <stdexcept>
#include <algorithm>
#include <cmath>
#include <chrono>
#include "imgui.h"

class WeaponViewer {
    template<class T> using Ptr=Microsoft::WRL::ComPtr<T>;
public:
    struct Vertex { float position[3],normal[3],uv[2]; unsigned char color[4]; };
    static_assert(sizeof(Vertex)==36,"Vertex asset layout");
private:
    struct Material {
        std::string name,relative,status="Original";
        Ptr<ID3D11ShaderResourceView> original,replacement;
        UINT width=1,height=1,hdWidth=0,hdHeight=0;
        uintmax_t pendingSize=0,loadedSize=0;
        std::filesystem::file_time_type pendingTime={},loadedTime={};
        int stable=0;
    };
    struct Draw { UINT first,count,material; };
    struct Mesh {
        std::string name,lod; Ptr<ID3D11Buffer> vb,ib;
        UINT vertices=0,indices=0;
        DirectX::XMFLOAT3 center; float radius=1;
        std::vector<Draw> draws;
    };
    struct Target {UINT width=0,height=0;Ptr<ID3D11Texture2D> color;Ptr<ID3D11RenderTargetView> rtv;Ptr<ID3D11DepthStencilView> depth;Ptr<ID3D11ShaderResourceView> image;};
    struct Constants {DirectX::XMFLOAT4X4 matrix;DirectX::XMFLOAT4 options;};
    Ptr<ID3D11Device> device;
    Ptr<ID3D11VertexShader> vs; Ptr<ID3D11PixelShader> ps; Ptr<ID3D11InputLayout> layout;
    Ptr<ID3D11Buffer> constants; Ptr<ID3D11SamplerState> sampler;
    Ptr<ID3D11RasterizerState> solid,wire;Ptr<ID3D11DepthStencilState> depth;
    Ptr<ID3D11BlendState> blend;Ptr<IWICImagingFactory> wic;
    std::vector<Mesh> meshes;std::vector<std::string> names;std::vector<Material> materials;
    std::map<std::string,UINT> materialIds;
    std::filesystem::path assets,rootPath;
    Target targets[2];
    int selected=0,textureMode=0,viewMode=0,atlasMaterial=0;
    bool low=false,wireframe=false,spinning=false,orbiting=false;
    float yaw=.5f,pitch=.22f,zoom=1;
    double nextPoll=0;
    char filter[128]={},rootText[2048]={};
    std::string message;
    static std::string Lower(std::string s){for(char& c:s)c=static_cast<char>(tolower(static_cast<unsigned char>(c)));return s;}
    static void Check(HRESULT h){if(FAILED(h))throw std::runtime_error("DX11/WIC error: "+std::to_string(static_cast<unsigned>(h)));}
    static std::vector<char> Read(const std::filesystem::path& p){
        std::ifstream f(p,std::ios::binary|std::ios::ate);if(!f)throw std::runtime_error("Cannot read "+p.u8string());
        auto n=f.tellg();if(n<=0 || n>128*1024*1024)throw std::runtime_error("Invalid asset length");
        std::vector<char> b(static_cast<size_t>(n));f.seekg(0);if(!f.read(b.data(),b.size()))throw std::runtime_error("Short asset read");return b;
    }
    static std::vector<std::vector<std::string>> Rows(const std::filesystem::path& p,bool required=true){
        std::ifstream f(p);if(!f && required)throw std::runtime_error("Cannot read "+p.u8string());
        std::vector<std::vector<std::string>> result;std::string line;
        while(std::getline(f,line)){if(!line.empty()&&line.back()=='\r')line.pop_back();if(line.empty()||line[0]=='#')continue;
            std::vector<std::string> fields;std::istringstream stream(line);std::string value;while(std::getline(stream,value,'\t'))fields.push_back(value);result.push_back(fields);}
        return result;
    }
    Ptr<ID3D11Buffer> Buffer(const std::vector<char>& bytes,UINT bind){D3D11_BUFFER_DESC d={};d.ByteWidth=(UINT)bytes.size();d.Usage=D3D11_USAGE_IMMUTABLE;d.BindFlags=bind;D3D11_SUBRESOURCE_DATA init={bytes.data(),0,0};Ptr<ID3D11Buffer> b;Check(device->CreateBuffer(&d,&init,&b));return b;}
    Ptr<ID3D11ShaderResourceView> Upload(const void* rgba,UINT w,UINT h){
        D3D11_TEXTURE2D_DESC d={};d.Width=w;d.Height=h;d.MipLevels=d.ArraySize=1;d.SampleDesc.Count=1;d.Format=DXGI_FORMAT_R8G8B8A8_UNORM;d.Usage=D3D11_USAGE_IMMUTABLE;d.BindFlags=D3D11_BIND_SHADER_RESOURCE;
        D3D11_SUBRESOURCE_DATA init={rgba,w*4,0};Ptr<ID3D11Texture2D> t;Ptr<ID3D11ShaderResourceView> s;Check(device->CreateTexture2D(&d,&init,&t));Check(device->CreateShaderResourceView(t.Get(),nullptr,&s));return s;
    }
    Ptr<ID3D11ShaderResourceView> LoadImage(const std::filesystem::path& p,UINT& width,UINT& height){
        // Snapshot the file, then decode completely before replacing a working SRV.
        auto bytes=Read(p);Ptr<IWICStream> stream;Check(wic->CreateStream(&stream));Check(stream->InitializeFromMemory((BYTE*)bytes.data(),(DWORD)bytes.size()));
        Ptr<IWICBitmapDecoder> decoder;Check(wic->CreateDecoderFromStream(stream.Get(),nullptr,WICDecodeMetadataCacheOnLoad,&decoder));
        Ptr<IWICBitmapFrameDecode> frame;Check(decoder->GetFrame(0,&frame));Check(frame->GetSize(&width,&height));
        if(!width||!height||width>8192||height>8192||uint64_t(width)*height>16777216)throw std::runtime_error("Replacement image exceeds 16M pixels");
        Ptr<IWICFormatConverter> convert;Check(wic->CreateFormatConverter(&convert));Check(convert->Initialize(frame.Get(),GUID_WICPixelFormat32bppRGBA,WICBitmapDitherTypeNone,nullptr,0,WICBitmapPaletteTypeCustom));
        std::vector<BYTE> rgba(size_t(width)*height*4);Check(convert->CopyPixels(nullptr,width*4,(UINT)rgba.size(),rgba.data()));return Upload(rgba.data(),width,height);
    }
    UINT MaterialId(const std::string& name){
        auto key=Lower(name);auto found=materialIds.find(key);if(found!=materialIds.end())return found->second;
        Material m;m.name=name;m.status="Missing in supplied texture archive";unsigned char pixel[]={180,180,180,255};m.original=Upload(pixel,1,1);
        UINT id=(UINT)materials.size();materials.push_back(std::move(m));materialIds[key]=id;return id;
    }
    void Resize(Target& t,UINT w,UINT h){
        w=std::clamp(w,1u,4096u);h=std::clamp(h,1u,4096u);if(t.width==w&&t.height==h)return;
        Target fresh;D3D11_TEXTURE2D_DESC d={};d.Width=w;d.Height=h;d.MipLevels=d.ArraySize=1;d.SampleDesc.Count=1;d.Format=DXGI_FORMAT_R8G8B8A8_UNORM;d.Usage=D3D11_USAGE_DEFAULT;d.BindFlags=D3D11_BIND_RENDER_TARGET|D3D11_BIND_SHADER_RESOURCE;
        Check(device->CreateTexture2D(&d,nullptr,&fresh.color));Check(device->CreateRenderTargetView(fresh.color.Get(),nullptr,&fresh.rtv));Check(device->CreateShaderResourceView(fresh.color.Get(),nullptr,&fresh.image));
        d.Format=DXGI_FORMAT_D24_UNORM_S8_UINT;d.BindFlags=D3D11_BIND_DEPTH_STENCIL;Ptr<ID3D11Texture2D> dt;Check(device->CreateTexture2D(&d,nullptr,&dt));Check(device->CreateDepthStencilView(dt.Get(),nullptr,&fresh.depth));fresh.width=w;fresh.height=h;t=std::move(fresh);
    }
    Mesh& Current(){const auto& name=names.at(selected);for(auto& m:meshes)if(m.name==name&&m.lod==(low?"low":"high"))return m;for(auto& m:meshes)if(m.name==name)return m;throw std::runtime_error("No model variant");}
    void ResetView(){yaw=.5f;pitch=.22f;zoom=.75f;orbiting=false;atlasMaterial=0;}
    void SaveSettings(){std::ofstream f(assets/"viewer-settings.txt");f<<rootPath.u8string()<<'\n';if(!f)message="Could not save settings";}
    void SaveMappings(){
        auto temp=assets/"replacements.tsv.tmp";std::ofstream f(temp);
        for(const auto& m:materials)if(!m.relative.empty())f<<m.name<<'\t'<<m.relative<<'\n';f.close();
        if(!f || !MoveFileExW(temp.c_str(),(assets/"replacements.tsv").c_str(),MOVEFILE_REPLACE_EXISTING|MOVEFILE_WRITE_THROUGH))message="Could not save mapping";
    }
    void ChooseReplacement(Material& m){
        wchar_t filename[32768]={};OPENFILENAMEW d={sizeof(d)};d.hwndOwner=GetActiveWindow();d.lpstrFilter=L"Images\0*.png;*.jpg;*.jpeg;*.bmp\0All files\0*.*\0";d.lpstrFile=filename;d.nMaxFile=32768;d.Flags=OFN_FILEMUSTEXIST|OFN_PATHMUSTEXIST|OFN_NOCHANGEDIR;
        if(!GetOpenFileNameW(&d))return;
        auto chosen=std::filesystem::path(filename);std::error_code ec;auto relative=std::filesystem::relative(chosen,rootPath,ec);
        m.relative=(!ec&&!relative.empty()&&*relative.begin()!=L"..")?relative.generic_u8string():chosen.u8string();m.replacement.Reset();m.loadedTime={};m.stable=0;SaveMappings();nextPoll=0;
    }
    std::vector<UINT> CurrentMaterials(){std::vector<UINT> result;for(auto& d:Current().draws)if(std::find(result.begin(),result.end(),d.material)==result.end())result.push_back(d.material);return result;}
    void ReadMappings(){
        std::map<std::string,std::string> mapping;for(auto& row:Rows(assets/"replacements.tsv",false))if(row.size()==2)mapping[Lower(row[0])]=row[1];
        for(auto& m:materials){std::string next=mapping[Lower(m.name)];if(next!=m.relative){m.relative=next;m.replacement.Reset();m.loadedTime={};m.stable=0;}}
    }
public:
    // Host must initialize COM on this thread (CoInitializeEx); no frame ownership.
    void Init(ID3D11Device* dev,const std::filesystem::path& assetDirectory){
        device=dev;assets=assetDirectory;
        Check(CoCreateInstance(CLSID_WICImagingFactory,nullptr,CLSCTX_INPROC_SERVER,IID_PPV_ARGS(&wic)));
        for(auto& r:Rows(assets/"textures.tsv")){
            if(r.size()!=4)throw std::runtime_error("Bad texture catalog");Material m;m.name=r[0];m.width=std::stoul(r[1]);m.height=std::stoul(r[2]);auto data=Read(assets/std::filesystem::u8path(r[3]));
            if(!m.width||!m.height||uint64_t(m.width)*m.height*4!=data.size())throw std::runtime_error("Bad texture dimensions");m.original=Upload(data.data(),m.width,m.height);materialIds[Lower(m.name)]=(UINT)materials.size();materials.push_back(std::move(m));
        }
        for(auto& r:Rows(assets/"catalog.tsv")){
            if(r.size()!=9)throw std::runtime_error("Bad model catalog");Mesh m;m.name=r[0];m.lod=r[1];m.vertices=std::stoul(r[3]);m.indices=std::stoul(r[4]);m.center={std::stof(r[5]),std::stof(r[6]),std::stof(r[7])};m.radius=std::stof(r[8]);
            auto vb=Read(assets/(r[2]+".vertices.bin"));auto ib=Read(assets/(r[2]+".indices.bin"));if(vb.size()!=size_t(m.vertices)*36||ib.size()!=size_t(m.indices)*4||m.indices%3)throw std::runtime_error("Bad mesh length");
            for(size_t at=0;at<ib.size();at+=4){UINT i;memcpy(&i,ib.data()+at,4);if(i>=m.vertices)throw std::runtime_error("Mesh index out of bounds");}
            m.vb=Buffer(vb,D3D11_BIND_VERTEX_BUFFER);m.ib=Buffer(ib,D3D11_BIND_INDEX_BUFFER);
            UINT covered=0;for(auto& d:Rows(assets/(r[2]+".draws.tsv"))){if(d.size()!=3)throw std::runtime_error("Bad draw record");UINT first=std::stoul(d[0]),count=std::stoul(d[1]);if(first!=covered||count%3||first>m.indices||count>m.indices-first)throw std::runtime_error("Bad draw range");m.draws.push_back({first,count,MaterialId(d[2])});covered+=count;}
            if(covered!=m.indices)throw std::runtime_error("Incomplete draw ranges");meshes.push_back(std::move(m));if(std::find(names.begin(),names.end(),r[0])==names.end())names.push_back(r[0]);
        }
        if(names.empty())throw std::runtime_error("Empty model catalog");std::sort(names.begin(),names.end());SelectModel("glock18");
        std::ifstream settings(assets/"viewer-settings.txt");std::string root;std::getline(settings,root);if(!root.empty()&&root.back()=='\r')root.pop_back();SetReplacementRoot(std::filesystem::u8path(root),false);ReadMappings();
        const char* shader=R"(
cbuffer Camera:register(b0){row_major float4x4 viewProjection;float4 options;};
Texture2D albedo:register(t0);SamplerState texSampler:register(s0);
struct Input{float3 p:POSITION;float3 n:NORMAL;float2 uv:TEXCOORD;float4 color:COLOR;};
struct Output{float4 p:SV_POSITION;float3 n:NORMAL;float2 uv:TEXCOORD;};
Output VSMain(Input i){Output o;o.p=mul(float4(i.p,1),viewProjection);o.n=i.n;o.uv=i.uv;return o;}
float4 PSMain(Output i):SV_TARGET{float4 c=albedo.Sample(texSampler,i.uv);clip(c.a-.02);float lighting=.55+.45*saturate(dot(normalize(i.n),normalize(float3(.3,.6,.8))));return float4(c.rgb*lighting*1.2,c.a);}
)";
        Ptr<ID3DBlob> v,p,error;HRESULT hr=D3DCompile(shader,strlen(shader),"WeaponViewer",nullptr,nullptr,"VSMain","vs_4_0",D3DCOMPILE_ENABLE_STRICTNESS,0,&v,&error);if(FAILED(hr))throw std::runtime_error(error?(char*)error->GetBufferPointer():"Vertex shader failure");
        hr=D3DCompile(shader,strlen(shader),"WeaponViewer",nullptr,nullptr,"PSMain","ps_4_0",D3DCOMPILE_ENABLE_STRICTNESS,0,&p,&error);if(FAILED(hr))throw std::runtime_error(error?(char*)error->GetBufferPointer():"Pixel shader failure");
        Check(device->CreateVertexShader(v->GetBufferPointer(),v->GetBufferSize(),nullptr,&vs));Check(device->CreatePixelShader(p->GetBufferPointer(),p->GetBufferSize(),nullptr,&ps));
        D3D11_INPUT_ELEMENT_DESC el[]={{"POSITION",0,DXGI_FORMAT_R32G32B32_FLOAT,0,0,D3D11_INPUT_PER_VERTEX_DATA,0},{"NORMAL",0,DXGI_FORMAT_R32G32B32_FLOAT,0,12,D3D11_INPUT_PER_VERTEX_DATA,0},{"TEXCOORD",0,DXGI_FORMAT_R32G32_FLOAT,0,24,D3D11_INPUT_PER_VERTEX_DATA,0},{"COLOR",0,DXGI_FORMAT_R8G8B8A8_UNORM,0,32,D3D11_INPUT_PER_VERTEX_DATA,0}};
        Check(device->CreateInputLayout(el,4,v->GetBufferPointer(),v->GetBufferSize(),&layout));
        D3D11_BUFFER_DESC cb={};cb.ByteWidth=sizeof(Constants);cb.Usage=D3D11_USAGE_DEFAULT;cb.BindFlags=D3D11_BIND_CONSTANT_BUFFER;Check(device->CreateBuffer(&cb,nullptr,&constants));
        D3D11_SAMPLER_DESC sd={};sd.Filter=D3D11_FILTER_MIN_MAG_MIP_LINEAR;sd.AddressU=sd.AddressV=sd.AddressW=D3D11_TEXTURE_ADDRESS_WRAP;sd.MaxLOD=D3D11_FLOAT32_MAX;sd.ComparisonFunc=D3D11_COMPARISON_NEVER;Check(device->CreateSamplerState(&sd,&sampler));
        D3D11_RASTERIZER_DESC rd={};rd.FillMode=D3D11_FILL_SOLID;rd.CullMode=D3D11_CULL_NONE;rd.DepthClipEnable=TRUE;Check(device->CreateRasterizerState(&rd,&solid));rd.FillMode=D3D11_FILL_WIREFRAME;Check(device->CreateRasterizerState(&rd,&wire));
        D3D11_DEPTH_STENCIL_DESC dd={};dd.DepthEnable=TRUE;dd.DepthWriteMask=D3D11_DEPTH_WRITE_MASK_ALL;dd.DepthFunc=D3D11_COMPARISON_LESS;Check(device->CreateDepthStencilState(&dd,&depth));
        D3D11_BLEND_DESC bd={};auto& bt=bd.RenderTarget[0];bt.BlendEnable=TRUE;bt.SrcBlend=D3D11_BLEND_SRC_ALPHA;bt.DestBlend=D3D11_BLEND_INV_SRC_ALPHA;bt.BlendOp=D3D11_BLEND_OP_ADD;bt.SrcBlendAlpha=D3D11_BLEND_ONE;bt.DestBlendAlpha=D3D11_BLEND_INV_SRC_ALPHA;bt.BlendOpAlpha=D3D11_BLEND_OP_ADD;bt.RenderTargetWriteMask=D3D11_COLOR_WRITE_ENABLE_ALL;Check(device->CreateBlendState(&bd,&blend));
    }
    bool SelectModel(const char* modelName){if(!modelName)return false;for(size_t i=0;i<names.size();i++)if(Lower(names[i])==Lower(modelName)){if(selected!=(int)i){selected=(int)i;ResetView();}return true;}return false;}
    void SetReplacementRoot(const std::filesystem::path& path,bool persist=true){rootPath=path;auto text=path.u8string();strncpy_s(rootText,text.c_str(),_TRUNCATE);for(auto& m:materials){m.replacement.Reset();m.stable=0;m.loadedTime={};}nextPoll=0;if(persist)SaveSettings();}
    void PollReplacements(bool force=false){
        double now=std::chrono::duration<double>(std::chrono::steady_clock::now().time_since_epoch()).count();if(!force&&now<nextPoll)return;nextPoll=now+1;ReadMappings();
        for(UINT id:CurrentMaterials()){
            auto& m=materials[id];if(m.relative.empty()){m.status=m.width==1?"Missing original; using neutral material":"No verified replacement mapping";continue;}
            auto p=std::filesystem::u8path(m.relative);if(p.is_relative()){if(rootPath.empty()){m.status="Set replacement folder";continue;}p=rootPath/p;}
            std::error_code ec;auto size=std::filesystem::file_size(p,ec);if(ec){m.replacement.Reset();m.loadedTime={};m.stable=0;m.status="Replacement missing; using original";continue;}
            auto time=std::filesystem::last_write_time(p,ec);if(ec||!size){m.status="Waiting for replacement file";continue;}
            if(m.replacement&&time==m.loadedTime&&size==m.loadedSize){m.status="Replacement loaded";continue;}
            if(time!=m.pendingTime||size!=m.pendingSize){m.pendingTime=time;m.pendingSize=size;m.stable=0;m.status="Waiting for file to finish";continue;}
            if(++m.stable<1)continue;
            try {UINT w,h;auto srv=LoadImage(p,w,h);if(m.width>1&&(w%m.width||h%m.height||w/m.width!=h/m.height))throw std::runtime_error("Replacement must preserve original dimensions/aspect");
                // Ensure the file did not change while WIC decoded it.
                if(std::filesystem::last_write_time(p)!=time||std::filesystem::file_size(p)!=size)throw std::runtime_error("File changed during load");
                m.replacement=std::move(srv);m.hdWidth=w;m.hdHeight=h;m.loadedTime=time;m.loadedSize=size;m.status="Replacement loaded";
            }catch(const std::exception& e){m.status=std::string(m.replacement?"Keeping last good texture: ":"Using original: ")+e.what();}
        }
    }
    // Rebind your application's 3D pipeline after this if rendering more scene content.
    void Render(ID3D11DeviceContext* ctx,int slot,UINT w,UINT h,bool useReplacement){
        using namespace DirectX;Target& t=targets[slot];Resize(t,w,h);Mesh& m=Current();
        Ptr<ID3D11RenderTargetView> oldRT;Ptr<ID3D11DepthStencilView> oldDepth;ctx->OMGetRenderTargets(1,&oldRT,&oldDepth);
        UINT nvp=D3D11_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE;D3D11_VIEWPORT vps[D3D11_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE];ctx->RSGetViewports(&nvp,vps);
        ID3D11ShaderResourceView* nil=nullptr;ctx->PSSetShaderResources(0,1,&nil);ctx->OMSetRenderTargets(1,t.rtv.GetAddressOf(),t.depth.Get());const float bg[]={.065f,.08f,.105f,1};ctx->ClearRenderTargetView(t.rtv.Get(),bg);ctx->ClearDepthStencilView(t.depth.Get(),D3D11_CLEAR_DEPTH|D3D11_CLEAR_STENCIL,1,0);
        D3D11_VIEWPORT vp={0,0,(float)t.width,(float)t.height,0,1};ctx->RSSetViewports(1,&vp);float aspect=float(t.width)/t.height;
        // Common camera center/radius across LODs avoids jumps when switching detail.
        Mesh* framing=&m;for(auto& candidate:meshes)if(candidate.name==m.name&&candidate.lod=="high")framing=&candidate;
        float radius=std::max(framing->radius,.05f);float distance=radius*3.15f*zoom*std::max(1.f,1.f/aspect);
        XMVECTOR target=XMLoadFloat3(&framing->center),eye=XMVectorAdd(target,XMVectorSet(sinf(yaw)*cosf(pitch)*distance,sinf(pitch)*distance,cosf(yaw)*cosf(pitch)*distance,0));Constants cb;
        XMStoreFloat4x4(&cb.matrix,XMMatrixLookAtRH(eye,target,XMVectorSet(0,1,0,0))*XMMatrixPerspectiveFovRH(XM_PIDIV4,aspect,radius*.01f,radius*200));cb.options=XMFLOAT4(0,0,0,0);ctx->UpdateSubresource(constants.Get(),0,nullptr,&cb,0,0);
        UINT stride=36,offset=0;ctx->IASetVertexBuffers(0,1,m.vb.GetAddressOf(),&stride,&offset);ctx->IASetIndexBuffer(m.ib.Get(),DXGI_FORMAT_R32_UINT,0);ctx->IASetInputLayout(layout.Get());ctx->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        ctx->VSSetShader(vs.Get(),nullptr,0);ctx->PSSetShader(ps.Get(),nullptr,0);ctx->GSSetShader(nullptr,nullptr,0);ctx->HSSetShader(nullptr,nullptr,0);ctx->DSSetShader(nullptr,nullptr,0);ctx->VSSetConstantBuffers(0,1,constants.GetAddressOf());ctx->PSSetConstantBuffers(0,1,constants.GetAddressOf());ctx->PSSetSamplers(0,1,sampler.GetAddressOf());ctx->RSSetState(wireframe?wire.Get():solid.Get());ctx->OMSetDepthStencilState(depth.Get(),0);ctx->OMSetBlendState(blend.Get(),nullptr,0xffffffff);
        for(auto& d:m.draws){auto& mat=materials[d.material];ID3D11ShaderResourceView* srv=useReplacement&&mat.replacement?mat.replacement.Get():mat.original.Get();ctx->PSSetShaderResources(0,1,&srv);ctx->DrawIndexed(d.count,d.first,0);}
        ctx->PSSetShaderResources(0,1,&nil);ctx->OMSetRenderTargets(1,oldRT.GetAddressOf(),oldDepth.Get());ctx->RSSetViewports(nvp,vps);
    }
    void DrawPanel(ID3D11DeviceContext* ctx,ImVec2 available=ImVec2(0,0)){
        if(available.x<=0||available.y<=0)available=ImGui::GetContentRegionAvail();PollReplacements();
        ImGui::PushID(this);ImGui::BeginChild("WeaponBrowser",available,false);
        ImGui::BeginChild("Models",ImVec2(210,0),true);
        ImGui::Text("%d models",(int)names.size());ImGui::SetNextItemWidth(-1);ImGui::InputTextWithHint("##filter","Find model...",filter,sizeof(filter));
        for(int i=0;i<(int)names.size();i++)if(Lower(names[i]).find(Lower(filter))!=std::string::npos)if(ImGui::Selectable(names[i].c_str(),selected==i)){selected=i;ResetView();nextPoll=0;}
        ImGui::EndChild();ImGui::SameLine();ImGui::BeginChild("Preview",ImVec2(0,0),false);
        ImGui::TextUnformatted(names[selected].c_str());ImGui::SameLine();ImGui::TextDisabled("%u vertices | %u triangles",Current().vertices,Current().indices/3);
        ImGui::SetNextItemWidth(150);ImGui::Combo("Textures",&textureMode,"Original\0Replacement\0Compare\0");ImGui::SameLine();ImGui::SetNextItemWidth(120);ImGui::Combo("View",&viewMode,"3D model\0Texture atlas\0");
        bool hasLow=false;for(auto& m:meshes)if(m.name==names[selected]&&m.lod=="low")hasLow=true;
        ImGui::BeginDisabled(!hasLow);ImGui::Checkbox("Low detail",&low);ImGui::EndDisabled();ImGui::SameLine();ImGui::Checkbox("Wireframe",&wireframe);ImGui::SameLine();ImGui::Checkbox("Rotate",&spinning);ImGui::SameLine();if(ImGui::Button("Reset view"))ResetView();
        if(ImGui::CollapsingHeader("Replacement textures / live reload")){
            ImGui::SetNextItemWidth(-110);ImGui::InputText("##root",rootText,sizeof(rootText));ImGui::SameLine();if(ImGui::Button("Apply folder"))SetReplacementRoot(std::filesystem::u8path(rootText));
            ImGui::TextWrapped("Original textures are used wherever a mapped replacement is unavailable. Completed file changes reload automatically.");
            auto ids=CurrentMaterials();atlasMaterial=std::clamp(atlasMaterial,0,(int)ids.size()-1);auto& mat=materials[ids[atlasMaterial]];
            if(ImGui::BeginCombo("Material",mat.name.c_str())){for(int i=0;i<(int)ids.size();i++)if(ImGui::Selectable(materials[ids[i]].name.c_str(),atlasMaterial==i))atlasMaterial=i;ImGui::EndCombo();}
            auto& chosen=materials[ids[atlasMaterial]];ImGui::TextWrapped("%s",chosen.status.c_str());if(!chosen.relative.empty())ImGui::TextWrapped("%s",chosen.relative.c_str());
            if(ImGui::Button("Choose replacement image..."))ChooseReplacement(chosen);ImGui::SameLine();if(ImGui::Button("Clear mapping")){chosen.relative.clear();chosen.replacement.Reset();SaveMappings();}
            if(!message.empty())ImGui::TextWrapped("%s",message.c_str());
        }
        ImGui::TextDisabled("Drag to orbit | Wheel to zoom | Both comparison panes share the camera");
        auto ids=CurrentMaterials();int loaded=0;for(UINT id:ids)if(materials[id].replacement)loaded++;
        ImGui::Text("Replacement coverage: %d / %d materials",loaded,(int)ids.size());
        ImVec2 area=ImGui::GetContentRegionAvail();int panes=textureMode==2?2:1;float paneWidth=(area.x-(panes-1)*8)/panes;
        bool hovered=false;
        if(paneWidth>=1&&area.y>=35){
            for(int slot=0;slot<panes;slot++){
                if(slot)ImGui::SameLine(0,8);ImGui::BeginChild(slot?"ReplacementPane":"OriginalPane",ImVec2(paneWidth,area.y),false);
                bool replacement=textureMode==1||(textureMode==2&&slot==1);ImGui::TextUnformatted(replacement?"Replacement (original fallback)":"Original");ImVec2 size=ImGui::GetContentRegionAvail();
                if(viewMode==0){Resize(targets[slot],(UINT)std::max(1.f,size.x),(UINT)std::max(1.f,size.y));ImGui::Image((ImTextureID)(intptr_t)targets[slot].image.Get(),size);hovered|=ImGui::IsItemHovered();}
                else{atlasMaterial=std::clamp(atlasMaterial,0,(int)ids.size()-1);auto& mat=materials[ids[atlasMaterial]];bool hd=replacement&&mat.replacement;float w=hd?(float)mat.hdWidth:(float)mat.width,h=hd?(float)mat.hdHeight:(float)mat.height;float fit=std::min(size.x/w,size.y/h);ImGui::Image((ImTextureID)(intptr_t)(hd?mat.replacement.Get():mat.original.Get()),ImVec2(w*fit,h*fit));}
                ImGui::EndChild();
            }
            if(hovered&&ImGui::IsMouseClicked(ImGuiMouseButton_Left))orbiting=true;if(!ImGui::IsMouseDown(ImGuiMouseButton_Left))orbiting=false;
            if(viewMode==0){if(orbiting){yaw-=ImGui::GetIO().MouseDelta.x*.01f;pitch=std::clamp(pitch+ImGui::GetIO().MouseDelta.y*.01f,-1.45f,1.45f);}if(hovered)zoom=std::clamp(zoom*expf(-ImGui::GetIO().MouseWheel*.12f),.65f,4.f);if(spinning)yaw+=ImGui::GetIO().DeltaTime*.5f;
                for(int slot=0;slot<panes;slot++)Render(ctx,slot,targets[slot].width,targets[slot].height,textureMode==1||(textureMode==2&&slot==1));}
        }
        ImGui::EndChild();ImGui::EndChild();ImGui::PopID();
    }
    void DrawWindow(ID3D11DeviceContext* ctx){auto* vp=ImGui::GetMainViewport();ImGui::SetNextWindowPos(vp->WorkPos);ImGui::SetNextWindowSize(vp->WorkSize);ImGui::Begin("GameZ Weapon Viewer",nullptr,ImGuiWindowFlags_NoDecoration|ImGuiWindowFlags_NoSavedSettings);DrawPanel(ctx);ImGui::End();}
    // Small widget for an existing weapon editor: no browser, Begin/End, or frame calls.
    // Call at most once per frame per WeaponViewer instance (targets are reused).
    // mode: 0 original, 1 replacement with original fallback, 2 side-by-side.
    void DrawSelected(ID3D11DeviceContext* ctx,ImVec2 size,int mode=1){
        spinning = true; if (zoom > .75f)zoom= .75f; float zoomMin=.65f;float zoomMax=4.f;
        if(size.x<4||size.y<4)return;int panes=mode==2?2:1;float w=(size.x-(panes-1)*8)/panes;if(w<1)return;PollReplacements();
        bool hover=false;for(int i=0;i<panes;i++){if(i)ImGui::SameLine(0,8);Resize(targets[i],(UINT)w,(UINT)size.y);ImGui::Image((ImTextureID)(intptr_t)targets[i].image.Get(),ImVec2(w,size.y));hover|=ImGui::IsItemHovered();}
        if(hover&&ImGui::IsMouseClicked(ImGuiMouseButton_Left))orbiting=true;if(!ImGui::IsMouseDown(ImGuiMouseButton_Left))orbiting=false;
        if(orbiting){yaw-=ImGui::GetIO().MouseDelta.x*.01f;pitch=std::clamp(pitch+ImGui::GetIO().MouseDelta.y*.01f,-1.45f,1.45f);}if(hover)zoom=std::clamp(zoom*expf(-ImGui::GetIO().MouseWheel*.12f),.20f,1.0f);if (spinning)yaw+=ImGui::GetIO().DeltaTime*.5f;
        for(int i=0;i<panes;i++)Render(ctx,i,targets[i].width,targets[i].height,mode==1||(mode==2&&i==1));
    }
    void SetLowDetail(bool enabled){low=enabled;}
    void Capture(ID3D11DeviceContext* ctx,const std::filesystem::path& path,ID3D11Texture2D* source=nullptr){
        if(!source)source=targets[0].color.Get();D3D11_TEXTURE2D_DESC d;source->GetDesc(&d);d.Usage=D3D11_USAGE_STAGING;d.BindFlags=0;d.CPUAccessFlags=D3D11_CPU_ACCESS_READ;Ptr<ID3D11Texture2D> staging;Check(device->CreateTexture2D(&d,nullptr,&staging));ctx->CopyResource(staging.Get(),source);D3D11_MAPPED_SUBRESOURCE mapped;Check(ctx->Map(staging.Get(),0,D3D11_MAP_READ,0,&mapped));std::ofstream out(path,std::ios::binary);out<<"P6\n"<<d.Width<<" "<<d.Height<<"\n255\n";for(UINT y=0;y<d.Height;y++)for(UINT x=0;x<d.Width;x++)out.write((char*)mapped.pData+y*mapped.RowPitch+x*4,3);ctx->Unmap(staging.Get(),0);if(!out)throw std::runtime_error("Capture failed");
    }
    void SelfTest(ID3D11DeviceContext* ctx,const std::filesystem::path& output){
        std::filesystem::create_directories(output);int num=0;
        for(auto& m:meshes){SelectModel(m.name.c_str());low=m.lod=="low";Render(ctx,0,480,320,false);Capture(ctx,output/(m.name+"_"+m.lod+".ppm"));num++;}
        SelectModel("glock18");low=false;PollReplacements(true);PollReplacements(true);Render(ctx,0,800,600,false);Capture(ctx,output/"original.ppm");Render(ctx,0,800,600,true);Capture(ctx,output/"replacement.ppm");wireframe=true;Render(ctx,0,640,480,false);Capture(ctx,output/"wireframe.ppm");wireframe=false;textureMode=2;std::ofstream report(output/"test-summary.txt");report<<num<<" model variants rendered\n";for(UINT id:CurrentMaterials())report<<materials[id].name<<": "<<materials[id].status<<'\n';
        // Exercise incomplete writes and replacement changes on isolated test files.
        UINT id=CurrentMaterials().front();auto& material=materials[id];auto savedAssets=assets,savedRoot=rootPath;
        auto replacement=std::filesystem::u8path(material.relative);if(replacement.is_relative())replacement=rootPath/replacement;
        bool hasHD=material.replacement!=nullptr;auto temporary=output/"reload-case";std::filesystem::create_directories(temporary);auto candidate=temporary/"candidate.png";
        std::filesystem::remove(candidate);std::ofstream(temporary/"replacements.tsv")<<material.name<<"\tcandidate.png\n";assets=temporary;rootPath=temporary;
        auto ensure=[](bool ok,const char* why){if(!ok)throw std::runtime_error(why);};
        PollReplacements(true);ensure(!material.replacement,"Missing replacement did not fall back");
        std::ofstream(candidate,std::ios::binary)<<"partial PNG";PollReplacements(true);PollReplacements(true);ensure(!material.replacement,"Partial PNG accepted");
        std::filesystem::copy_file(hasHD?replacement:savedAssets/"textures/glock18.tif.png",candidate,std::filesystem::copy_options::overwrite_existing);PollReplacements(true);PollReplacements(true);ensure(material.replacement!=nullptr,"Complete PNG not loaded");auto good=material.replacement;
        std::ofstream(candidate,std::ios::binary)<<"partial update";PollReplacements(true);PollReplacements(true);ensure(material.replacement==good,"Last good texture lost on partial update");
        std::filesystem::copy_file(savedAssets/"textures/glock18.tif.png",candidate,std::filesystem::copy_options::overwrite_existing);PollReplacements(true);PollReplacements(true);ensure(material.replacement&&material.hdWidth==64,"Changed PNG not reloaded");
        std::filesystem::remove(candidate);PollReplacements(true);ensure(!material.replacement,"Deleted texture did not fall back");
        assets=savedAssets;rootPath=savedRoot;ReadMappings();PollReplacements(true);PollReplacements(true);report<<"Live reload: missing / partial / valid / partial update / changed / deleted PASS\n";
    }
};
