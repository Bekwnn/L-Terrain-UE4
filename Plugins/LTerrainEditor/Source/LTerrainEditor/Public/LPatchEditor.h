#pragma once
#include "LTerrainEditor.h"
#include "LNoise.h"
#include "SlateCore.h"

class SLPatchView;
class SLNoiseView;
class SLGroundTexView;
class SLScatterView;

//Spawns the rule editor tab and ui
class SLPatchEditor : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SLPatchEditor) {}

	SLATE_END_ARGS()

	void Construct(const FArguments& args);

	FReply OnAddPatchClicked();
	FReply OnRemovePatchClicked();
	TSharedRef<ITableRow> GenerateListRow(LPatchPtr item, const TSharedRef<STableViewBase> &ownerTable);
	void SelectionChanged(LPatchPtr item, ESelectInfo::Type selectType);

public:
	TSharedPtr<FLTerrainEditorModule> lTerrainModule;

protected:
	TSharedPtr<SListView<LPatchPtr>> patchListWidget;
	TSharedPtr<SLPatchView> patchViewWidget;
};

class SLPatchView : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SLPatchView) {}
	SLATE_ARGUMENT(LPatchPtr, Patch)
	SLATE_END_ARGS()

	void Construct(const FArguments& args);
	void Reconstruct(LPatchPtr item);

	FReply OnNoiseAdded();
	FReply OnNoiseRemoved();
	TSharedRef<ITableRow> GenerateNoiseListRow(LNoisePtr item, const TSharedRef<STableViewBase> &ownerTable);
	void NoiseSelectionChanged(LNoisePtr item, ESelectInfo::Type selectType);

	FReply OnGroundTexAdded();
	FReply OnGroundTexRemoved();
	TSharedRef<ITableRow> GenerateGroundTexListRow(LGroundTexturePtr item, const TSharedRef<STableViewBase> &ownerTable);
	void GroundTexSelectionChanged(LGroundTexturePtr item, ESelectInfo::Type selectType);

	FReply OnScatterAdded();
	FReply OnScatterRemoved();
	TSharedRef<ITableRow> GenerateScatterListRow(LObjectScatterPtr item, const TSharedRef<STableViewBase> &ownerTable);
	void ScatterSelectionChanged(LObjectScatterPtr item, ESelectInfo::Type selectType);

private:
	LPatchPtr patch;

	TSharedPtr<SListView<LNoisePtr>> noiseListWidget;
	TSharedPtr<SLNoiseView> noiseView;

	TSharedPtr<SListView<LGroundTexturePtr>> groundTexListWidget;
	TSharedPtr<SLGroundTexView> groundTexView;

	TSharedPtr<SListView<LObjectScatterPtr>> scatterListWidget;
	TSharedPtr<SLScatterView> scatterView;
};

class SLNoiseView : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SLNoiseView) {}
	SLATE_ARGUMENT(LNoisePtr, Noise)
	SLATE_END_ARGS()

	void Construct(const FArguments& args);
	void Reconstruct(LNoisePtr item);

	void NoiseTypeSelectionChanged(TSharedPtr<FString> string, ESelectInfo::Type selectType);

public:
	static TArray<TSharedPtr<FString>> noiseNames;
};

class SLGroundTexView : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SLGroundTexView) {}
	SLATE_ARGUMENT(LGroundTexturePtr, GroundTexture)
	SLATE_END_ARGS()

	void Construct(const FArguments& args);
	void Reconstruct(LGroundTexturePtr item);
};

class SLScatterView : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SLScatterView) {}
	SLATE_ARGUMENT(LObjectScatterPtr, ObjectScatter)
	SLATE_END_ARGS()

	void Construct(const FArguments& args);
	void Reconstruct(LObjectScatterPtr item);
};