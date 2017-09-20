#include "LTerrainEditor.h"
#include "LGenOptions.h"
#include "LTerrainGeneration.h"
#include "Kismet/GameplayStatics.h"
#include "Editor.h"

#define LOCTEXT_NAMESPACE "FLTerrainEditorModule"

void SLGenOptions::Construct(const FArguments & args)
{
	lTerrainModule = FLTerrainEditorModule::GetModule();

	ChildSlot
	[
		SNew(SHorizontalBox)
		+ SHorizontalBox::Slot()
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot()
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				.AutoWidth()
				[
					SNew(SVerticalBox)
					+ SVerticalBox::Slot()
					.Padding(2)
					.FillHeight(1)
					[
						SAssignNew(groundTexList, SListView<LGroundTexturePtr>)
						.ListItemsSource(&(lTerrainModule->lSystem.groundTextures))
						.OnGenerateRow(this, &SLGenOptions::GenerateGroundTexListRow)
						.OnSelectionChanged(this, &SLGenOptions::GroundTexSelectionChanged)
						.SelectionMode(ESelectionMode::Single)
					]
					+ SVerticalBox::Slot()
					.Padding(2)
					.AutoHeight()
					[
						SNew(SButton)
						.Text(LOCTEXT("NewRuleButton", "+ Add Layer"))
						.OnClicked(FOnClicked::CreateRaw(this, &SLGenOptions::OnAddGroundTexClicked))
					]
					+ SVerticalBox::Slot()
					.Padding(2)
					.AutoHeight()
					[
						SNew(SButton)
						.Text(LOCTEXT("DelRuleButton", "- Remove Selected Layer"))
						.OnClicked(FOnClicked::CreateRaw(this, &SLGenOptions::OnRemoveGroundTexClicked))
					]
				]
				+ SHorizontalBox::Slot()
				.FillWidth(1)
				[
					SNew(SBorder)
					[
						SAssignNew(groundTexView, SLGroundTexView)
						.GroundTexture(nullptr)
					]
				]
			]
			+ SVerticalBox::Slot()
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				.AutoWidth()
				[
					SNew(SVerticalBox)
					+ SVerticalBox::Slot()
					.Padding(2)
					.FillHeight(1)
					[
						SAssignNew(objectList, SListView<LMeshAssetPtr>)
						.ListItemsSource(&(lTerrainModule->lSystem.meshAssets))
						.OnGenerateRow(this, &SLGenOptions::GenerateObjectListRow)
						.OnSelectionChanged(this, &SLGenOptions::ObjectSelectionChanged)
						.SelectionMode(ESelectionMode::Single)
					]
					+ SVerticalBox::Slot()
					.Padding(2)
					.AutoHeight()
					[
						SNew(SButton)
						.Text(LOCTEXT("NewRuleButton", "+ Add Mesh Asset"))
						.OnClicked(FOnClicked::CreateRaw(this, &SLGenOptions::OnAddMeshAssetClicked))
					]
					+ SVerticalBox::Slot()
					.Padding(2)
					.AutoHeight()
					[
						SNew(SButton)
						.Text(LOCTEXT("DelRuleButton", "- Remove Mesh Asset"))
						.OnClicked(FOnClicked::CreateRaw(this, &SLGenOptions::OnRemoveMeshAssetClicked))
					]
				]
				+ SHorizontalBox::Slot()
				.FillWidth(1)
				[
					SNew(SBorder)
					[
						SAssignNew(meshAssetView, SLMeshAssetView)
						.MeshAsset(nullptr)
					]
				]
			]
			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				SNew(SButton)
				.Text(LOCTEXT("GenTerrainButton", "+ Generate Terrain"))
				.OnClicked_Raw(this, &SLGenOptions::OnGenerateClicked)
			]
		]
	];
}

TSharedRef<ITableRow> SLGenOptions::GenerateGroundTexListRow(LGroundTexturePtr item, const TSharedRef<STableViewBase>& ownerTable)
{
	return SNew(STableRow<LGroundTexturePtr>, ownerTable)
		.Padding(2)
		[
			SNew(STextBlock)
			.Text(FText::FromString(item->name))
		];
}

void SLGenOptions::GroundTexSelectionChanged(LGroundTexturePtr item, ESelectInfo::Type selectType)
{
	groundTexView->Reconstruct(item);
}

TSharedRef<ITableRow> SLGenOptions::GenerateObjectListRow(LMeshAssetPtr item, const TSharedRef<STableViewBase>& ownerTable)
{
	return SNew(STableRow<LMeshAssetPtr>, ownerTable)
		.Padding(2)
		[
			SNew(STextBlock)
			.Text(FText::FromString(item->name))
		];
}

void SLGenOptions::ObjectSelectionChanged(LMeshAssetPtr item, ESelectInfo::Type selectType)
{
	meshAssetView->Reconstruct(item);
}

//any final checks or tweaks to the lSystem before calling the terrain generation function go here
FReply SLGenOptions::OnGenerateClicked()
{
	TArray<AActor*> actors;
	UWorld* world = GEditor->GetEditorWorldContext().World();
	UGameplayStatics::GetAllActorsOfClass(world, ALandscape::StaticClass(), actors);
	ALandscape* landscape = (actors.Num() > 0)? Cast<ALandscape>(actors[0]) : nullptr;
	if (lTerrainModule->lSystem.lSystemLoDs.Num() > 0 && landscape != nullptr)
	{
		LTerrainGeneration::GenerateTerrain(lTerrainModule->lSystem, landscape);
	}
	return FReply::Handled();
}

FReply SLGenOptions::OnAddGroundTexClicked()
{
	LGroundTexturePtr newGroundTex = LGroundTexturePtr(new LGroundTexture());
	newGroundTex->name = "New Ground Tex";

	lTerrainModule->lSystem.groundTextures.Add(newGroundTex);

	groundTexList->SetSelection(newGroundTex);
	groundTexList->RequestListRefresh();

	return FReply::Handled();
}

FReply SLGenOptions::OnRemoveGroundTexClicked()
{
	TArray<LGroundTexturePtr> selectedItems = groundTexList->GetSelectedItems();

	for (LGroundTexturePtr item : selectedItems)
	{
		lTerrainModule->lSystem.groundTextures.Remove(item);
	}

	groundTexList->ClearSelection();
	groundTexList->RequestListRefresh();

	return FReply::Handled();
}

FReply SLGenOptions::OnAddMeshAssetClicked()
{
	LMeshAssetPtr newMeshAsset = LMeshAssetPtr(new LMeshAsset());
	newMeshAsset->name = "New Mesh Asset";

	lTerrainModule->lSystem.meshAssets.Add(newMeshAsset);

	objectList->SetSelection(newMeshAsset);
	objectList->RequestListRefresh();

	return FReply::Handled();
}

FReply SLGenOptions::OnRemoveMeshAssetClicked()
{
	TArray<LMeshAssetPtr> selectedItems = objectList->GetSelectedItems();

	for (LMeshAssetPtr item : selectedItems)
	{
		lTerrainModule->lSystem.meshAssets.Remove(item);
	}

	objectList->ClearSelection();
	objectList->RequestListRefresh();

	return FReply::Handled();
}


void SLGroundTexView::Construct(const FArguments& args)
{
	if (!args._GroundTexture.IsValid()) return;

	Reconstruct(args._GroundTexture);
}

void SLGroundTexView::Reconstruct(LGroundTexturePtr item)
{
	if (!item.IsValid()) return;

	ChildSlot
	[
		SNew(SVerticalBox)
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(2)
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.Padding(2)
			[
				SNew(STextBlock)
				.Text(FText::FromString("Layer Name"))
			]
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.Padding(2)
			[
				SNew(SEditableTextBox)
				.MinDesiredWidth(200)
				.Text_Lambda([item]()->FText {
					return FText::FromString(item->name);
				})
				.OnTextChanged_Lambda([item](FText newText) {
					item->name = newText.ToString();
				})
			]
		]
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(2)
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.AutoWidth()
			[
				SNew(SObjectPropertyEntryBox) //TODO change to class with thumbnail preview
				.AllowedClass(UTexture2D::StaticClass())
				.OnObjectChanged_Lambda([item](FAssetData newTexture) {
					item->texture = newTexture;
				})
				.ObjectPath_Lambda([item]()->FString {
					return item->texture.ObjectPath.ToString();
				})
			]
			+ SHorizontalBox::Slot()
			.AutoWidth()
			[
				SNew(SObjectPropertyEntryBox) //TODO change to class with thumbnail preview
				.AllowedClass(UTexture2D::StaticClass())
				.OnObjectChanged_Lambda([item](FAssetData newTexture) {
					item->normalMap = newTexture;
				})
				.ObjectPath_Lambda([item]()->FString {
					return item->normalMap.ObjectPath.ToString();
				})
			]
		]
	];
}

void SLMeshAssetView::Construct(const FArguments& args)
{
	if (!args._MeshAsset.IsValid()) return;

	Reconstruct(args._MeshAsset);
}

void SLMeshAssetView::Reconstruct(LMeshAssetPtr item)
{
	if (!item.IsValid()) return;

	ChildSlot
	[
		SNew(SVerticalBox)
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(2)
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.Padding(2)
			[
				SNew(STextBlock)
				.Text(FText::FromString("Object Name"))
			]
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.Padding(2)
			[
				SNew(SEditableTextBox)
				.MinDesiredWidth(200)
				.Text_Lambda([item]()->FText {
					return FText::FromString(item->name);
				})
				.OnTextChanged_Lambda([item](FText newText) {
					item->name = newText.ToString();
				})
			]
		]
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(2)
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.AutoWidth()
			[
				SNew(SObjectPropertyEntryBox) //TODO change to class with thumbnail preview
				.AllowedClass(UStaticMesh::StaticClass())
				.OnObjectChanged_Lambda([item](FAssetData newMesh) {
					item->object = newMesh;
				})
				.ObjectPath_Lambda([item]()->FString {
					return item->object.ObjectPath.ToString();
				})
			]
		]
	];
}

#undef LOCTEXT_NAMESPACE