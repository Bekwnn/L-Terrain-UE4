#pragma once
#include "LTerrainEditor.h"
#include "SlateCore.h"

class SLGroundTexView;
class SLMeshAssetView;

//Spawns the rule editor tab and ui
class SLGenOptions : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SLGenOptions) {}

	SLATE_END_ARGS()

	void Construct(const FArguments& args);

	TSharedRef<ITableRow> GenerateGroundTexListRow(LGroundTexturePtr item, const TSharedRef<STableViewBase>& ownerTable);
	void GroundTexSelectionChanged(LGroundTexturePtr item, ESelectInfo::Type selectType);

	TSharedRef<ITableRow> GenerateObjectListRow(LMeshAssetPtr item, const TSharedRef<STableViewBase>& ownerTable);
	void ObjectSelectionChanged(LMeshAssetPtr item, ESelectInfo::Type selectType);

	FReply OnGenerateClicked();

	FReply OnAddGroundTexClicked();
	FReply OnRemoveGroundTexClicked();

	FReply OnAddMeshAssetClicked();
	FReply OnRemoveMeshAssetClicked();
	
public:
	TSharedPtr<FLTerrainEditorModule> lTerrainModule;
	TSharedPtr<SListView<LGroundTexturePtr>> groundTexList;
	TSharedPtr<SListView<LMeshAssetPtr>> objectList;

	TSharedPtr<SLGroundTexView> groundTexView;
	TSharedPtr<SLMeshAssetView> meshAssetView;
};

class SLGroundTexView : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SLGroundTexView) {}
	SLATE_ARGUMENT(LGroundTexturePtr, GroundTexture)
	SLATE_END_ARGS()

	void Construct(const FArguments& args);

	void Reconstruct(LGroundTexturePtr item);

	FReply CreateNewLandscapeLayerInfo(LGroundTexturePtr viewItem);
};

class SLMeshAssetView : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SLMeshAssetView) {}
	SLATE_ARGUMENT(LMeshAssetPtr, MeshAsset)
	SLATE_END_ARGS()

	void Construct(const FArguments& args);

	void Reconstruct(LMeshAssetPtr item);

	FReply CreateNewFoliageType(LMeshAssetPtr viewItem);
};

