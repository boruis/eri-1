/*
 *  scene_actor.h
 *  eri
 *
 *  Created by exe on 11/29/09.
 *  Copyright 2009 cobbler. All rights reserved.
 *
 */

#ifndef ERI_SCENE_ACTOR_H
#define ERI_SCENE_ACTOR_H

#include "math_helper.h"
#include "render_data.h"
#include "material_data.h"

#include <string>
#include <vector>

#ifdef OS_ANDROID
#include <GLES/gl.h>
#else
#import <OpenGLES/ES1/gl.h>
#import <OpenGLES/ES2/gl.h>
#endif

namespace ERI {
	
	class Renderer;
	
	struct UserData
	{
		virtual ~UserData() {}
	};
	
#pragma mark SceneActor
	
	class SceneActor
	{
	public:
		SceneActor();
		virtual ~SceneActor();
		
		// scene
		
		virtual void AddToScene(int layer_id = 0);
		virtual void RemoveFromScene();

		void MoveToLayer(int layer_id);
		
		void AddChild(SceneActor* actor);
		void RemoveChild(SceneActor* actor);
		
		// intersection
		
		bool IsHit(const Vector3& world_space_pos);
		SceneActor* GetHitActor(const Vector3& parent_space_pos);
		
		// render
				
		virtual void Render(Renderer* renderer);

		void SetColor(const Color& color);
		const Color& GetColor();
		
		void BlendAdd();
		
		//
		// transform
		//
		
		const Matrix4& GetTransform();
		const Matrix4& GetInvTransform();
		const Matrix4& GetWorldTransform();
		
		// TODO: 2D & 3D transform interface is suck, re-design
		
		// 2D
		virtual void SetPos(float x, float y);
		Vector2 GetPos();
				
		void SetRotate(float degree);
		float GetRotate();
		
		void SetScale(float x, float y);
		Vector2 GetScale();
		
		// 3D
		virtual void SetPos(const Vector3& pos);
		const Vector3& GetPos3();
		void SetRotate(float degree, const Vector3& axis);
		void SetScale(const Vector3& scale);
		
		// material
		// TODO: bad interface, re-design
		
		inline void set_accept_light(bool accept) { material_data_.accept_light = accept; }
		
		void SetMaterial(const std::string& texture_path, TextureFilter filter_min = FILTER_NEAREST, TextureFilter filter_mag = FILTER_NEAREST);
		void SetMaterial(const Texture* tex, TextureFilter filter_min = FILTER_NEAREST, TextureFilter filter_mag = FILTER_NEAREST);
		void SetTextureFilter(TextureFilter filter_min, TextureFilter filter_mag);
		
		void SetOpacityType(OpacityType type);
		inline OpacityType opacity_type() { return material_data_.opacity_type; }
		
		void SetDepthTest(bool enable);
		void SetDepthWrite(bool enable);

		inline const MaterialData& material() { return material_data_; }

		//

		inline void set_visible(bool visible) { visible_ = visible; }
		inline bool visible() { return visible_; }
		
		inline void set_user_data(UserData* data) { user_data_ = data; }
		inline UserData* user_data() { return user_data_; }
		
		inline int layer_id() { return layer_id_; }
			
	protected:
		virtual bool IsInArea(const Vector3& local_space_pos) { return false; }
		
		void SetTransformDirty();
		void SetWorldTransformDirty();
		
		void SetTexture(const Texture* tex);
		
		RenderData		render_data_;
		MaterialData	material_data_;

		int				layer_id_;
		
		SceneActor*					parent_;
		std::vector<SceneActor*>	childs_;

		bool			visible_;
		
		UserData*		user_data_;
	};
	
#pragma mark CameraActor
	
	class CameraActor : public SceneActor
	{
	public:
		enum Projection
		{
			ORTHOGONAL,
			PERSPECTIVE
		};
		
		CameraActor(Projection projection = ORTHOGONAL);
		virtual ~CameraActor();
		
		virtual void SetPos(float x, float y);
		virtual void SetPos(const Vector3& pos);
		
		void SetLookAt(const Vector3& look_at, bool is_offset = false);
		
		void SetOrthoZoom(float zoom);
		void SetPerspectiveFov(float fov_y);

		void UpdateViewMatrix();
		void UpdateProjectionMatrix();
		
		void SetViewProjectionModified();
		void SetViewModified();
		void SetProjectionModified();
		
		inline Projection projection() { return projection_; }
		inline float ortho_zoom() { return ortho_zoom_; }
		inline float perspective_fov() { return perspective_fov_y_; }
		inline bool is_view_modified() { return is_view_modified_; }
		inline bool is_projection_modified() { return is_projection_modified_; }
		
	private:
		Projection	projection_;
		
		Vector3		look_at_;
		bool		is_look_at_offset_;
		
		float		ortho_zoom_;
		float		perspective_fov_y_;
		
		bool		is_view_modified_;
		bool		is_projection_modified_;
	};
	
#pragma mark LightActor
	
	class LightActor : public SceneActor
	{
	public:
		enum Type
		{
			POINT,
			DIRECTION,
			SPOT
		};
		
		LightActor(Type type);
		virtual ~LightActor();
		
		virtual void AddToScene(int layer_id = 0);
		virtual void RemoveFromScene();
		
		virtual void SetPos(float x, float y);
		virtual void SetPos(const Vector3& pos);

		void SetDir(const Vector3& dir);
		
		void SetAmbient(const Color& ambient);
		void SetDiffuse(const Color& diffuse);
		void SetSpecular(const Color& specular);
		
		void SetAttenuation(float constant, float linear, float quadratic);
		
		void SetSpotExponent(float exponent);
		void SetSpotCutoff(float cutoff);
		
	private:
		Type	type_;
		int		idx_;
		Color	ambient_, diffuse_, specular_;
		float	attenuation_constant_, attenuation_linear_, attenuation_quadratic_;
		
		Vector3	dir_;
		float	spot_exponent_, spot_cutoff_;
	};
	
#pragma mark SpriteActor
	
	class SpriteActor : public SceneActor
	{
	public:
		SpriteActor(float width, float height, float offset_width = 0.0f, float offset_height = 0.0f);
		SpriteActor(const std::string& txt, const std::string& font_name, float font_size, float width, float height, float offset_width = 0.0f, float offset_height = 0.0f);
		
		virtual ~SpriteActor();
		
		void SetSizeOffset(float width, float height, float offset_width = 0.0f, float offset_height = 0.0f);
		
		void SetTexScale(float u_scale, float v_scale);
		void SetTexScroll(float u_scroll, float v_scroll);
		void SetTexArea(int start_x, int start_y, int width, int height);
		void SetTexAreaUV(float start_u, float start_v, float width, float height);

		void SetUseLine(bool use_line);
		
	private:
		virtual bool IsInArea(const Vector3& local_space_pos);
		
		void UpdateVertexBuffer();

		vertex_2_pos_tex	vertices_[4];
		
		Vector2		size_;
		Vector2		offset_;
		Vector2		tex_scale_;
		Vector2		tex_scroll_;
		bool		is_use_line_;
	};
	
#pragma mark BoxActor
	
	class BoxActor : public SceneActor
	{
	public:
		BoxActor(const Vector3& half_ext);
		virtual ~BoxActor();
		
	private:
		void UpdateVertexBuffer();
		
		Vector3		half_ext_;
	};
	
#pragma mark NumberActor
	
	class NumberActor : public SceneActor
	{
	public:
		NumberActor(float width, float height, bool is_force_sign = true);
		NumberActor(float width, float height, const std::string& material, int tex_unit_width, int tex_unit_height, bool is_force_sign = true);
		virtual ~NumberActor();
		
		void SetTexUnit(int tex_unit_width, int tex_unit_height);
		
		void SetUseLine(bool use_line);
		
		void SetNumber(int number);
		void SetNumberFloat(float number);
		
		int number() { return number_; }
		int now_len() { return now_len_; }
	
	private:
		virtual bool IsInArea(const Vector3& local_space_pos);
		
		void UpdateVertexBuffer();
		
		vertex_2_pos_tex*	vertices_;
		int					now_len_max_;
		int					now_len_;
		
		Vector2		size_;
		Vector2		unit_uv_;
		bool		is_use_line_;
		
		int			number_;
		float		number_f_;
		bool		is_float_;
		bool		is_force_sign_;
	};
	
#pragma mark TxtActor
	
	class Font;
	
	class TxtActor : public SceneActor
	{
	public:
		TxtActor(const std::string& txt, const std::string& font_name, int font_size, bool is_pos_center = false);
		virtual ~TxtActor();
		
		void SetTxt(const std::string& txt);

		inline void set_area_border(float size) { border_size_ = size; }
		
		inline float width() { return width_; }
		inline float height() { return height_; }
		
		inline const std::string& txt() { return txt_; }
		
		static void CalculateSize(const std::string& txt, const Font* font, int font_size, float& width, float& height);
		
	private:
		virtual bool IsInArea(const Vector3& local_space_pos);
		
		void UpdateVertexBuffer();
		
		vertex_2_pos_tex*	vertices_;
		int					now_len_max_;
		int					now_len_;
		
		const Font*		font_;
		int				font_size_;
		bool			is_use_line_;
		
		float			width_, height_;
		float			border_size_;
		bool			is_pos_center_;
		
		std::string		txt_;
	};

}

#endif // ERI_SCENE_ACTOR_H