//
// Copyright (c) 2008-2020 the Urho3D project.
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
//

#include "../Precompiled.h"

#include "../Core/Context.h"
#include "../Graphics/Material.h"
#include "../Graphics/Texture2D.h"
#include "../Resource/ResourceCache.h"
#include "../Scene/Scene.h"
#include "../Urho2D/Renderer2D.h"
#include "../Urho2D/Sprite2D.h"
#include "MyStaticSprite2D.h"
#include "Urho3D/Urho3DAll.h"
#include "../DebugNew.h"

namespace Urho3D
{

	extern const char* URHO2D_CATEGORY;
	extern const char* blendModeNames[];

	MultiStaticSprite2D::MultiStaticSprite2D(Context* context) :
		Drawable2D(context),
		blendMode_(BLEND_ALPHA),
		flipX_(false),
		flipY_(false),
		swapXY_(false),
		color_(Color::WHITE),
		useHotSpot_(false),
		useDrawRect_(false),
		useTextureRect_(false),
		hotSpot_(0.5f, 0.5f),
		drawRect_(Rect::ZERO),
		textureRect_(Rect::ZERO)
	{
		SetBatchNum(2);
	}

	MultiStaticSprite2D::~MultiStaticSprite2D()
	{
		if (gridData_)free(gridData_);
	}

	void MultiStaticSprite2D::RegisterObject(Context* context)
	{
		context->RegisterFactory<MultiStaticSprite2D>(URHO2D_CATEGORY);

		URHO3D_ACCESSOR_ATTRIBUTE("Is Enabled", IsEnabled, SetEnabled, bool, true, AM_DEFAULT);
		URHO3D_COPY_BASE_ATTRIBUTES(Drawable2D);
		URHO3D_ENUM_ACCESSOR_ATTRIBUTE("Blend Mode", GetBlendMode, SetBlendMode, BlendMode, blendModeNames, BLEND_ALPHA, AM_DEFAULT);
		URHO3D_ACCESSOR_ATTRIBUTE("Flip X", GetFlipX, SetFlipX, bool, false, AM_DEFAULT);
		URHO3D_ACCESSOR_ATTRIBUTE("Flip Y", GetFlipY, SetFlipY, bool, false, AM_DEFAULT);
		URHO3D_ACCESSOR_ATTRIBUTE("Color", GetColor, SetColor, Color, Color::WHITE, AM_DEFAULT);
		URHO3D_MIXED_ACCESSOR_ATTRIBUTE("Custom material", GetCustomMaterialAttr, SetCustomMaterialAttr, ResourceRef,
			ResourceRef(Material::GetTypeStatic()), AM_DEFAULT);
	}

	void MultiStaticSprite2D::SetBatchNum(int num)
	{
		sourceBatches_.Resize(num);
		sprites_.Resize(num);
		for (int i = 0; i < num; i++)
		{
			sourceBatches_[i].owner_ = this;
			sprites_[i] = nullptr;
		}
	}

	void MultiStaticSprite2D::SetSprite(int i, Sprite2D* sprite)
	{
		if (sprite == sprites_[i])
			return;

		sprites_[i] = sprite;
		UpdateMaterial(i);

		sourceBatchesDirty_ = true;
		MarkNetworkUpdate();

		UpdateDrawRect(i);
	}

	void MultiStaticSprite2D::SetDrawRect(const Rect& rect)
	{
		drawRect_ = rect;

		if (useDrawRect_)
		{
			sourceBatchesDirty_ = true;
		}
	}

	void MultiStaticSprite2D::SetTextureRect(const Rect& rect)
	{
		textureRect_ = rect;

		if (useTextureRect_)
		{
			sourceBatchesDirty_ = true;
		}
	}

	void MultiStaticSprite2D::SetBlendMode(BlendMode blendMode)
	{
		if (blendMode == blendMode_)
			return;

		blendMode_ = blendMode;

		for (int i = 0; i < sprites_.Size(); i++)
			UpdateMaterial(i);
		MarkNetworkUpdate();
	}

	void MultiStaticSprite2D::SetFlip(bool flipX, bool flipY, bool swapXY)
	{
		if (flipX == flipX_ && flipY == flipY_ && swapXY == swapXY_)
			return;

		flipX_ = flipX;
		flipY_ = flipY;
		swapXY_ = swapXY;
		sourceBatchesDirty_ = true;

		MarkNetworkUpdate();
	}

	void MultiStaticSprite2D::SetFlipX(bool flipX)
	{
		SetFlip(flipX, flipY_, swapXY_);
	}

	void MultiStaticSprite2D::SetFlipY(bool flipY)
	{
		SetFlip(flipX_, flipY, swapXY_);
	}

	void MultiStaticSprite2D::SetSwapXY(bool swapXY)
	{
		SetFlip(flipX_, flipY_, swapXY);
	}

	void MultiStaticSprite2D::SetColor(const Color& color)
	{
		if (color == color_)
			return;

		color_ = color;
		sourceBatchesDirty_ = true;
		MarkNetworkUpdate();
	}

	void MultiStaticSprite2D::SetAlpha(float alpha)
	{
		if (alpha == color_.a_)
			return;

		color_.a_ = alpha;
		sourceBatchesDirty_ = true;
		MarkNetworkUpdate();
	}

	void MultiStaticSprite2D::SetUseHotSpot(bool useHotSpot)
	{
		if (useHotSpot == useHotSpot_)
			return;

		useHotSpot_ = useHotSpot;
		sourceBatchesDirty_ = true;
		MarkNetworkUpdate();

		for (int i = 0; i < sprites_.Size(); i++)
			UpdateDrawRect(i);
	}

	void MultiStaticSprite2D::SetUseDrawRect(bool useDrawRect)
	{
		if (useDrawRect == useDrawRect_)
			return;

		useDrawRect_ = useDrawRect;
		sourceBatchesDirty_ = true;
		MarkNetworkUpdate();

		for (int i = 0; i < sprites_.Size(); i++)
			UpdateDrawRect(i);
	}

	void MultiStaticSprite2D::SetUseTextureRect(bool useTextureRect)
	{
		if (useTextureRect == useTextureRect_)
			return;

		useTextureRect_ = useTextureRect;
		sourceBatchesDirty_ = true;
		MarkNetworkUpdate();
	}

	void MultiStaticSprite2D::SetHotSpot(const Vector2& hotspot)
	{
		if (hotspot == hotSpot_)
			return;

		hotSpot_ = hotspot;

		if (useHotSpot_)
		{
			sourceBatchesDirty_ = true;
			MarkNetworkUpdate();
		}

		for (int i = 0; i < sprites_.Size(); i++)
			UpdateDrawRect(i);
	}

	void MultiStaticSprite2D::SetCustomMaterial(Material* customMaterial)
	{
		if (customMaterial == customMaterial_)
			return;

		customMaterial_ = customMaterial;
		sourceBatchesDirty_ = true;

		for (int i = 0; i < sprites_.Size(); i++)
			UpdateMaterial(i);
		MarkNetworkUpdate();
	}

	Material* MultiStaticSprite2D::GetCustomMaterial() const
	{
		return customMaterial_;
	}

	void MultiStaticSprite2D::SetCustomMaterialAttr(const ResourceRef& value)
	{
		auto* cache = GetSubsystem<ResourceCache>();
		SetCustomMaterial(cache->GetResource<Material>(value.name_));
	}

	ResourceRef MultiStaticSprite2D::GetCustomMaterialAttr() const
	{
		return GetResourceRef(customMaterial_, Material::GetTypeStatic());
	}

	void MultiStaticSprite2D::OnSceneSet(Scene* scene)
	{
		Drawable2D::OnSceneSet(scene);


		for (int i = 0; i < sprites_.Size(); i++)
			UpdateMaterial(i);
	}

	void MultiStaticSprite2D::OnWorldBoundingBoxUpdate()
	{
		boundingBox_.Clear();
		worldBoundingBox_.Clear();

		const Vector<SourceBatch2D>& sourceBatches = GetSourceBatches();
		for (unsigned i = 0; i < sourceBatches[0].vertices_.Size(); ++i)
			worldBoundingBox_.Merge(sourceBatches[0].vertices_[i].position_);

		boundingBox_ = worldBoundingBox_.Transformed(node_->GetWorldTransform().Inverse());
	}

	void MultiStaticSprite2D::OnDrawOrderChanged()
	{
		sourceBatches_[0].drawOrder_ = GetDrawOrder();
	}

	void MultiStaticSprite2D::UpdateSourceBatches()
	{
		

		if (!sourceBatchesDirty_)
			return;

		Vector<Vector<Vertex2D>*> vers;
		vers.Resize(sourceBatches_.Size());
		for (int i = 0; i < sourceBatches_.Size(); i++)
		{
			vers[i] = &sourceBatches_[i].vertices_;
			vers[i]->Clear();
		}

		if (!sprites_[0])
			return;

		if (!useTextureRect_)
		{
			if (!sprites_[0]->GetTextureRectangle(textureRect_, flipX_, flipY_))
				return;
		}

		if (dataType == DataType::CharArray && gridData_ != nullptr)
		{
			int* wh = (int*)gridData_;
			char* data = gridData_ + 8;

			int width = wh[0], height = wh[1];
			for (int i = 0; i < width ; i++)for (int j =0; j < height; j++)
			{
				int x = i - width / 2;
				int y = j - height / 2;
				/*
				V1---------V2
				|         / |
				|       /   |
				|     /     |
				|   /       |
				| /         |
				V0---------V3
				*/
				Vertex2D vertex0;
				Vertex2D vertex1;
				Vertex2D vertex2;
				Vertex2D vertex3;

				// Convert to world space
				const int worldTransform = 1;
				//const Matrix3x4& worldTransform = node_->GetWorldTransform();
				vertex0.position_ = worldTransform * Vector3(drawRect_.min_.x_ + (float)x, drawRect_.min_.y_ + (float)y, 0.0f);
				vertex1.position_ = worldTransform * Vector3(drawRect_.min_.x_ + (float)x, drawRect_.max_.y_ + (float)y, 0.0f);
				vertex2.position_ = worldTransform * Vector3(drawRect_.max_.x_ + (float)x, drawRect_.max_.y_ + (float)y, 0.0f);
				vertex3.position_ = worldTransform * Vector3(drawRect_.max_.x_ + (float)x, drawRect_.min_.y_ + (float)y, 0.0f);

				vertex0.uv_ = textureRect_.min_;
				(swapXY_ ? vertex3.uv_ : vertex1.uv_) = Vector2(textureRect_.min_.x_, textureRect_.max_.y_);
				vertex2.uv_ = textureRect_.max_;
				(swapXY_ ? vertex1.uv_ : vertex3.uv_) = Vector2(textureRect_.max_.x_, textureRect_.min_.y_);

				vertex0.color_ = vertex1.color_ = vertex2.color_ = vertex3.color_ = color_.ToUInt();

				auto n = data[i * height + j];
				if (n >= 0 && n < sourceBatches_.Size())
				{


				vers[n]->Push(vertex0);
				vers[n]->Push(vertex1);
				vers[n]->Push(vertex2);
				vers[n]->Push(vertex3);
				}
			}

		}
		else if(dataType == DataType::List)
		{
			for (auto p : points_)
			{
				{
					float i = p.x_ , j = p.y_;
					/*
					V1---------V2
					|         / |
					|       /   |
					|     /     |
					|   /       |
					| /         |
					V0---------V3
					*/
					Vertex2D vertex0;
					Vertex2D vertex1;
					Vertex2D vertex2;
					Vertex2D vertex3;

					// Convert to world space
					const int worldTransform = 1;
					//const Matrix3x4& worldTransform = node_->GetWorldTransform();
					vertex0.position_ = worldTransform * Vector3(drawRect_.min_.x_ + (float)i, drawRect_.min_.y_ + (float)j, 0.0f);
					vertex1.position_ = worldTransform * Vector3(drawRect_.min_.x_ + (float)i, drawRect_.max_.y_ + (float)j, 0.0f);
					vertex2.position_ = worldTransform * Vector3(drawRect_.max_.x_ + (float)i, drawRect_.max_.y_ + (float)j, 0.0f);
					vertex3.position_ = worldTransform * Vector3(drawRect_.max_.x_ + (float)i, drawRect_.min_.y_ + (float)j, 0.0f);

					vertex0.uv_ = textureRect_.min_;
					(swapXY_ ? vertex3.uv_ : vertex1.uv_) = Vector2(textureRect_.min_.x_, textureRect_.max_.y_);
					vertex2.uv_ = textureRect_.max_;
					(swapXY_ ? vertex1.uv_ : vertex3.uv_) = Vector2(textureRect_.max_.x_, textureRect_.min_.y_);

					vertex0.color_ = vertex1.color_ = vertex2.color_ = vertex3.color_ = color_.ToUInt();

					/*	if (Random(2) == 0)
						{

							vertices.Push(vertex0);
							vertices.Push(vertex1);
							vertices.Push(vertex2);
							vertices.Push(vertex3);
						}
						else
						{

							verteces2.Push(vertex0);
							verteces2.Push(vertex1);
							verteces2.Push(vertex2);
							verteces2.Push(vertex3);
						}*/
					int n = 0;// Random((int)vers.Size());
					vers[n]->Push(vertex0);
					vers[n]->Push(vertex1);
					vers[n]->Push(vertex2);
					vers[n]->Push(vertex3);
				}
			}
		}

		sourceBatchesDirty_ = false;
	}

	void MultiStaticSprite2D::UpdateMaterial(int i)
	{
		if (customMaterial_)
			sourceBatches_[i].material_ = customMaterial_;
		else
		{
			if (sprites_[i] && renderer_)
				sourceBatches_[i].material_ = renderer_->GetMaterial(sprites_[i]->GetTexture(), blendMode_);
			else
				sourceBatches_[i].material_ = nullptr;
		}
	}

	void MultiStaticSprite2D::UpdateDrawRect(int i)
	{
		if (!useDrawRect_)
		{
			if (useHotSpot_)
			{
				if (sprites_[i] && !sprites_[i]->GetDrawRectangle(drawRect_, hotSpot_, flipX_, flipY_))
					return;
			}
			else
			{
				if (sprites_[i] && !sprites_[i]->GetDrawRectangle(drawRect_, flipX_, flipY_))
					return;
			}
		}
	}

}
