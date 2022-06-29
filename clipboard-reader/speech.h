#pragma once

class CSpeech
{
private:
	std::string m_message;
	
public:
	CSpeech(std::string message) : m_message(message)
	{
		HRESULT hr = CoInitializeEx(NULL, COINIT_MULTITHREADED);
		if (FAILED(hr))
		{
			throw std::runtime_error("CoInitialize Failed");
		}
	}

	~CSpeech() throw()
	{
		CoUninitialize();
	}

	LRESULT TextToSpeech()
	{	
		ISpVoice * pVoice = NULL;
		
		auto hr = CoCreateInstance(CLSID_SpVoice, NULL, CLSCTX_ALL, IID_ISpVoice, (void **)&pVoice);
		if (SUCCEEDED(hr))
		{
			std::wstring nws = std::wstring(m_message.begin(), m_message.end());
			hr = pVoice->Speak(nws.c_str(), 0, NULL);
			
			pVoice->Release();
			pVoice = NULL;
		}

		return TRUE;
	}
};
