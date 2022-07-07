#pragma once

class CSpeech
{
	std::string message_;
	
public:
	explicit CSpeech(std::string message);

	~CSpeech() noexcept;

	LRESULT TextToSpeech();
};

inline CSpeech::CSpeech(std::string message): message_(std::move(message))
{
	const HRESULT hr = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
	if (FAILED(hr))
	{
		throw std::runtime_error("CoInitialize Failed");
	}
}

inline CSpeech::~CSpeech() noexcept
{
	CoUninitialize();
}

inline LRESULT CSpeech::TextToSpeech()
{	
	ISpVoice * p_voice = nullptr;

	const auto hr = CoCreateInstance(CLSID_SpVoice, nullptr, CLSCTX_ALL, IID_ISpVoice, reinterpret_cast<void**>(&p_voice));
	if (SUCCEEDED(hr))
	{
		const auto nws = std::wstring(message_.begin(), message_.end());
		p_voice->Speak(nws.c_str(), 0, nullptr);			
		p_voice->Release();
		p_voice = nullptr;
	}

	return TRUE;
}
