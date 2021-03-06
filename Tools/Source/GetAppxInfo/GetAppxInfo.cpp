// GetAppxInfo.cpp : Defines the entry point for the console application.
//

#include <iostream>
#include <iomanip>
#include <string>

#include <Windows.h>

#include <wrl\client.h>
#include <wrl\wrappers\corewrappers.h>

#include <AppxPackaging.h>
#include <msopc.h>

#define RETURN_IF_FAILED(hr) do { HRESULT __hrRet = (hr); if (FAILED(__hrRet)) { return __hrRet; }} while (0, 0)

using namespace std;

using namespace Microsoft::WRL;
using namespace Microsoft::WRL::Wrappers;

const int column1 = 20;

HRESULT GetPackageIdFromAppxInternal(const wstring& packagePath)
{
    if (packagePath.length() == 0)
    {
        return E_INVALIDARG;
    }

    HRESULT hr = CoInitializeEx(NULL, COINIT_MULTITHREADED);
    if (FAILED(hr))
    {
        wcout << L"CoInitializeEx failed" << endl;
        return hr;
    }

    ComPtr<IAppxManifestProperties> properties;
    ComPtr<IAppxManifestPackageId> packageId;
    ComPtr<IAppxManifestApplicationsEnumerator> applications;
    ComPtr<IAppxFactory> appxFactory;
    ComPtr<IOpcFactory> opcFactory;
    ComPtr<IStream> packageStream;
    ComPtr<IAppxPackageReader> packageReader;
    ComPtr<IAppxManifestReader> manifestReader;
    ComPtr<IAppxManifestApplication> application;

    // read the manifest
    RETURN_IF_FAILED(CoCreateInstance(__uuidof(AppxFactory), NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(appxFactory.GetAddressOf())));
    RETURN_IF_FAILED(CoCreateInstance(__uuidof(OpcFactory), NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(opcFactory.GetAddressOf())));
    RETURN_IF_FAILED(opcFactory->CreateStreamOnFile(packagePath.c_str(), OPC_STREAM_IO_READ, NULL, FILE_ATTRIBUTE_NORMAL, &packageStream));
    RETURN_IF_FAILED(appxFactory->CreatePackageReader(packageStream.Get(), &packageReader));
    RETURN_IF_FAILED(packageReader->GetManifest(&manifestReader));

    // get the manifest info
    RETURN_IF_FAILED(manifestReader->GetProperties(properties.GetAddressOf()));
    RETURN_IF_FAILED(manifestReader->GetPackageId(packageId.GetAddressOf()));
    RETURN_IF_FAILED(manifestReader->GetApplications(applications.GetAddressOf()));
    RETURN_IF_FAILED(applications->GetCurrent(application.GetAddressOf()));

    LPWSTR value;

    RETURN_IF_FAILED(packageId->GetName(&value));
    wcout << setw(column1) << left << L"Name" << L" : " << value << endl;
    CoTaskMemFree(value);

    RETURN_IF_FAILED(packageId->GetPublisher(&value));
    wcout << setw(column1) << left << L"Publisher" << L" : " << value << endl;
    CoTaskMemFree(value);

    APPX_PACKAGE_ARCHITECTURE architecture;
    RETURN_IF_FAILED(packageId->GetArchitecture(&architecture));
    switch (architecture)
    {
        case APPX_PACKAGE_ARCHITECTURE_ARM:
            wcout << setw(column1) << left << L"Architecture" << L" : " << L"ARM" << endl;
            break;

        //case APPX_PACKAGE_ARCHITECTURE_ARM64:
        //    wcout << setw(column1) << left << L"Architecture" << L" : " << L"ARM64" << endl;
        //    break;

        case APPX_PACKAGE_ARCHITECTURE_X64:
            wcout << setw(column1) << left << L"Architecture" << L" : " << L"X64" << endl;
            break;

        case APPX_PACKAGE_ARCHITECTURE_X86:
            wcout << setw(column1) << left << L"Architecture" << L" : " << L"X86" << endl;
            break;

        case APPX_PACKAGE_ARCHITECTURE_NEUTRAL:
            wcout << setw(column1) << left << L"Architecture" << L" : " << L"Neutral" << endl;
            break;
    }

    RETURN_IF_FAILED(packageId->GetResourceId(&value));
    wcout << setw(column1) << left << L"ResourceId" << L" : " << (value?value:L" ") << endl;
    CoTaskMemFree(value);

    UINT64 version;
    RETURN_IF_FAILED(packageId->GetVersion(&version));
    WORD major    = static_cast<WORD>((version & 0xFFFF000000000000ui64) >> 48);
    WORD minor    = static_cast<WORD>((version & 0x0000FFFF00000000ui64) >> 32);
    WORD build    = static_cast<WORD>((version & 0x00000000FFFF0000ui64) >> 16);
    WORD revision = static_cast<WORD>((version & 0x000000000000FFFFui64));
    wcout << setw(column1) << left << L"Version" << L" : " << major << L"." << minor << L"." << build << L"." << revision << endl;

    RETURN_IF_FAILED(packageId->GetPackageFullName(&value));
    wcout << setw(column1) << left << L"PackageFullName" << L" : " << value << endl;
    CoTaskMemFree(value);

    // InstallLocation
    // IsFramework

    RETURN_IF_FAILED(packageId->GetPackageFamilyName(&value));
    wcout << setw(column1) << left << L"PackageFamilyName" << L" : " << value << endl;
    CoTaskMemFree(value);

    hr = properties->GetStringValue(L"DisplayName", &value);
    if (SUCCEEDED(hr))
    {
        wcout << setw(column1) << left << L"DisplayName" << L" : " << value << endl;
        CoTaskMemFree(value);
    }

    hr = properties->GetStringValue(L"PublisherDisplayName", &value);
    if (SUCCEEDED(hr))
    {
        wcout << setw(column1) << left << L"PublisherDisplayName" << L" : " << value << endl;
        CoTaskMemFree(value);
    }

    hr = properties->GetStringValue(L"Logo", &value);
    if (SUCCEEDED(hr))
    {
        wcout << setw(column1) << left << L"Logo" << L" : " << value << endl;
        CoTaskMemFree(value);
    }

    RETURN_IF_FAILED(application->GetAppUserModelId(&value));
    wcout << setw(column1) << left << L"AppUserModelId" << L" : " << value << endl;
    CoTaskMemFree(value);

    // Capabilities?
    // DeviceCapabilities?
    // PackageDependencies?
    // Prerequisites? (OSMinVersion, OSMaxVersionTested)

    return S_OK;
}

int wmain(int argc, wchar_t *argv[])
{
    if (argc < 2)
    {
        wcout << "GetAppxInfo <path to appx file>" << endl;
        return -1;
    }

    HRESULT hr = GetPackageIdFromAppxInternal(argv[1]);
    if (FAILED(hr))
    {
        wcout << L"ERROR: " << std::hex << hr << endl;
        return hr;
    }
    return 0;
}

