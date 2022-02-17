#pragma once
#include "CoreMinimal.h"
#include "HAL/Runnable.h"

class PROCGENPROJECT_API FCustomWorker : public FRunnable
{
public:
	FCustomWorker();
	virtual ~FCustomWorker();

	//Overridden from parent
	bool Init() override; //Setup
	uint32 Run() override; //Main
	void Stop() override; //Clean
private:
	FRunnableThread* Thread;
	bool RunThread;
	bool InputReady;
};
