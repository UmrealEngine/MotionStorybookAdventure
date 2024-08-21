// Fill out your copyright notice in the Description page of Project Settings.


#include "BS_ServerManager.h"
#include "BS_Utility.h"
#include "Misc/Paths.h"
#include "HAL/PlatformProcess.h"
#include "Runtime/Networking/Public/Networking.h"
#include "Sockets.h"
#include "SocketSubsystem.h"
#include "Interfaces/IPv4/IPv4Address.h" 

// Sets default values
ABS_ServerManager::ABS_ServerManager()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// 기본 경로 가져오기
	pyDefaultPath = FPaths::ProjectContentDir() + TEXT("Scripts/");
	pyNames.SetNum(4);
}

// Called when the game starts or when spawned
void ABS_ServerManager::BeginPlay()
{
	Super::BeginPlay();
	
	// ReqPostTemp();
}

// Called every frame
void ABS_ServerManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// tcp 통신 결과 받기
	ReceiveData();
}



#pragma region 미사용

// void ABS_ServerManager::ReqGetValue(EQTEType type)
// {
// 	FString resultUrl = defaultUrl;
// 	// 해당 타입 찾아서 url 완성
// 	for(auto urlName : urlNames)
// 	{
// 		if(urlName.type == type)
// 		{
// 			resultUrl += urlName.urlName;
// 			break;
// 		}
// 	}

// 	// 1. http 모듈 생성
// 	FHttpModule* http = &FHttpModule::Get();

// 	// 2. 요청 정보 설정
// 	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> httpReq = http->CreateRequest();

// 	// 3. 응답 함수 연결
// 	httpReq->OnProcessRequestComplete().BindUObject(this, &ABS_ServerManager::ResGetValue);
// 	httpReq->SetURL(resultUrl);
// 	httpReq->SetVerb(TEXT("GET")); // == METHOD
// 	httpReq->SetHeader(TEXT("Content-Type"), TEXT("application/json")); // 헤더 설정 json

// 	// 서버 요청
// 	httpReq->ProcessRequest();


// }

// void ABS_ServerManager::ResGetValue(FHttpRequestPtr req, FHttpResponsePtr res, bool isSuccess)
// {
// 	// 응답성공
// 	if(isSuccess && res.IsValid())
// 	{
// 		FString resStr = res->GetContentAsString();
// 		// @@ 해당 값 가져오기
// 		// @@ 값으로 뭔가 하기

// 		// FString result = UJK_JsonLib::JsonParse(resStr);
// 		// FString result = resStr;

// 		GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Green, FString::Printf(TEXT("Response : %s"), *resStr));

// 		// mainUI->SetLogText(result);
// 	}
// 	// 응답실패
// 	else
// 	{
// 		GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Green, TEXT("요청 실패"));
// 	}
// }

// void ABS_ServerManager::ResPostTemp(FHttpRequestPtr req, FHttpResponsePtr res, bool isSuccess)
// {
// 	// 응답성공
// 	if(isSuccess && res.IsValid())
// 	{
// 		FString resStr = res->GetContentAsString();

// 		// FString result = UJK_JsonLib::JsonParse(resStr);
// 		// FString result = resStr;

// 		GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Green, FString::Printf(TEXT("Response : %s"), *resStr));

// 		// mainUI->SetLogText(result);
// 	}
// 	// 응답실패
// 	else
// 	{
// 		GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Green, TEXT("요청 실패"));
// 	}
// }

#pragma endregion

void ABS_ServerManager::RunQTEScript(EQTEType type)
{
	// 해당 스크립트 이름 찾아서 실행
	for(auto pyName : pyNames)
	{
		if(pyName.type == type)
		{
			FString pyPath = pyDefaultPath + pyName.pyName;
			RunPythonScript(pyPath);
			break;
		}
	}
}

void ABS_ServerManager::CreateClient()
{
    // 클라이언트 소켓 생성
    ClientSocket = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->CreateSocket(NAME_Stream, TEXT("TCP Client"), false);
    ServerAddr = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->CreateInternetAddr();

    // 서버 IP와 포트 설정
    FIPv4Address Ip;
    FIPv4Address::Parse(serverIP, Ip); // 서버의 IP 주소 (예: localhost)
    ServerAddr->SetIp(Ip.Value); // IP 주소 설정
    ServerAddr->SetPort(serverPort); // 서버 포트:

    // 서버에 연결
	FString connStr = ClientSocket->Connect(*ServerAddr) ? TEXT("서버에 연결되었습니다.") : TEXT("서버 연결 실패.");

	GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Green, FString::Printf(TEXT("%s"), *connStr));
}

void ABS_ServerManager::ReceiveData()
{
    if (ClientSocket)
    {
		GEngine->AddOnScreenDebugMessage(-1, -1.f, FColor::Green, TEXT("소켓 있음"));

		// 데이터 있으면 가져오기
        uint32 Size;
        while (ClientSocket->HasPendingData(Size))
        {
            TArray<uint8> ReceivedData;
            ReceivedData.SetNumUninitialized(FMath::Min(Size, 1024u));

            // 데이터 수신
            int32 BytesRead = 0;
            ClientSocket->Recv(ReceivedData.GetData(), ReceivedData.Num(), BytesRead);

            FString ReceivedString = FString(ANSI_TO_TCHAR(reinterpret_cast<const char*>(ReceivedData.GetData())));
			GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Green, FString::Printf(TEXT("jk_____Received: %s"), *ReceivedString));
			// @@ 이 데이터를 사용하기
			
        }
    }
	else
	{
		GEngine->AddOnScreenDebugMessage(-1, -1.f, FColor::Green, TEXT("소켓 없음"));
	}
}

void ABS_ServerManager::Disconnect()
{
	if(ClientSocket)
	{
		ClientSocket->Close();
		ClientSocket = nullptr;
		GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Green, TEXT("서버와 연결 종료"));
	}
	else
	{
		GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Green, TEXT("소켓 없음"));
	}
	
}