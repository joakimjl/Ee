// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "EeStructs.h"

FEeTargetData::FEeTargetData(int32 Index, int32 SerialNumber, const FTransform& Transform)
	: EntityNumber(Index)
	, EntitySerial(SerialNumber)
	, TargetTransform(Transform)
{
}

