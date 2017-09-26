#include "LTerrainEditor.h"
#include "LGenOptions.h"
#include "LTerrainGeneration.h"
#include "Kismet/GameplayStatics.h"
#include "Editor.h"
#include "LandscapeLayerInfoObject.h"
#include "Dialogs/DlgPickAssetPath.h"
#include "AssetRegistryModule.h"

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
						.Text(LOCTEXT("NewLayerButton", "+ Add Layer"))
						.OnClicked(FOnClicked::CreateRaw(this, &SLGenOptions::OnAddGroundTexClicked))
					]
					+ SVerticalBox::Slot()
					.Padding(2)
					.AutoHeight()
					[
						SNew(SButton)
						.Text(LOCTEXT("DelLayerButton", "- Remove Selected Layer"))
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
						.Text(LOCTEXT("AddMeshButton", "+ Add Mesh Asset"))
						.OnClicked(FOnClicked::CreateRaw(this, &SLGenOptions::OnAddMeshAssetClicked))
					]
					+ SVerticalBox::Slot()
					.Padding(2)
					.AutoHeight()
					[
						SNew(SButton)
						.Text(LOCTEXT("DelMeshButton", "- Remove Mesh Asset"))
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
			.Text_Lambda([item]()->FText{
				return FText::FromString(item->name);
			})
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
			.Text_Lambda([item]()->FText{
				return FText::FromString(item->name);
			})
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
	newGroundTex->name = "Ground Tex " + FString::FromInt(FLTerrainEditorModule::GetModule()->lSystem.groundTextures.Num());

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

	newMeshAsset->name = "Mesh Asset " + FString::FromInt(FLTerrainEditorModule::GetModule()->lSystem.meshAssets.Num());

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

///GENOPTIONS END
///GROUNDTEXVIEW BEGIN

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
			.Padding(2)
			[
				SNew(STextBlock)
				.Text(LOCTEXT("LayerInfoLabel", "Layer Info Object"))
			]
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.Padding(2)
			[
				SNew(SObjectPropertyEntryBox)
				.AllowedClass(ULandscapeLayerInfoObject::StaticClass())
				.OnObjectChanged_Lambda([item](FAssetData newInfo) {
					item->layerInfo = newInfo;
				})
				.ObjectPath_Lambda([item]()->FString {
					return item->layerInfo.ObjectPath.ToString();
				})
			]
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.Padding(2)
			[
				SNew(SButton)
				.OnClicked_Raw(this, &SLGroundTexView::CreateNewLandscapeLayerInfo, item)
				.Text(LOCTEXT("NewLayerAssetButton", "New"))
			]
		]
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(2)
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.Padding(2)
			[
				SNew(SVerticalBox)
				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(2)
				[
					SNew(STextBlock)
					.Text(LOCTEXT("DiffuseLabel", "Texture"))
				]
				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(2)
				[
					SNew(SObjectPropertyEntryBox)
					.AllowedClass(UTexture2D::StaticClass())
					.OnObjectChanged_Lambda([item](FAssetData newTexture) {
						item->texture = newTexture;
					})
					.ObjectPath_Lambda([item]()->FString {
						return item->texture.ObjectPath.ToString();
					})
				]
			]
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.Padding(2)
			[
				SNew(SVerticalBox)
				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(2)
				[
					SNew(STextBlock)
					.Text(LOCTEXT("NormMapLabel", "Normal Map"))
				]
				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(2)
				[
					SNew(SObjectPropertyEntryBox)
					.AllowedClass(UTexture2D::StaticClass())
					.OnObjectChanged_Lambda([item](FAssetData newTexture) {
						item->normalMap = newTexture;
					})
					.ObjectPath_Lambda([item]()->FString {
						return item->normalMap.ObjectPath.ToString();
					})
				]
			]
		]
	];
}

FReply SLGroundTexView::CreateNewLandscapeLayerInfo(LGroundTexturePtr viewItem)
{
	FName layerName = FName(*viewItem->name);
	FName layerObjectName = FName(*FString::Printf(TEXT("%s_LayerInfo"), *layerName.ToString()));

	FString path = TEXT("/Game/LTerrain_assets/");
	FString packageName = path + layerObjectName.ToString();

	TSharedRef<SDlgPickAssetPath> NewLayerDlg =
		SNew(SDlgPickAssetPath)
		.Title(LOCTEXT("CreateNewLayerInfo", "Create New Landscape Layer Info Object"))
		.DefaultAssetPath(FText::FromString(packageName));

	if (NewLayerDlg->ShowModal() != EAppReturnType::Cancel)
	{
		packageName = NewLayerDlg->GetFullAssetPath().ToString();
		layerObjectName = FName(*NewLayerDlg->GetAssetName().ToString());

		UPackage* package = CreatePackage(NULL, *packageName);
		ULandscapeLayerInfoObject* newInfo = NewObject<ULandscapeLayerInfoObject>(package, layerObjectName, RF_Public | RF_Standalone | RF_Transactional);
		newInfo->LayerName = layerName;
		newInfo->bNoWeightBlend = false;

		//notify asset registry
		FAssetRegistryModule::AssetCreated(newInfo);

		//mark the package dirty
		package->MarkPackageDirty();

		//show in content browser
		TArray<UObject*> objects;
		objects.Add(newInfo);
		GEditor->SyncBrowserToObjects(objects);
	}

	return FReply::Handled();
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