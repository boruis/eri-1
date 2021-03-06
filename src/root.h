/*
 *  root.h
 *  eri
 *
 *  Created by exe on 11/28/09.
 *  Copyright 2009 cobbler. All rights reserved.
 *
 */

#ifndef ERI_ROOT_H
#define ERI_ROOT_H

#include <cstddef>

namespace ERI {
	
	class Renderer;
	class SceneMgr;
	class InputMgr;
	class TextureMgr;
	class FontMgr;
	class ShaderMgr;
	
	class Root
	{
	public:
		Root();
		~Root();
		
		void Init(bool use_depth_buffer = true);
		void Update();
		
		inline Renderer* renderer() { return renderer_; }
		inline SceneMgr* scene_mgr() { return scene_mgr_; }
		inline InputMgr* input_mgr() { return input_mgr_; }
		inline TextureMgr* texture_mgr() { return texture_mgr_; }
		inline FontMgr* font_mgr() { return font_mgr_; }
		inline ShaderMgr* shader_mgr() { return shader_mgr_; }

		void* window_handle() { return window_handle_; }
		inline void set_window_handle(void* handle) { window_handle_ = handle; }
	
		inline static Root& Ins()
		{
			if (!ins_ptr_)	ins_ptr_ = new Root();
			return *ins_ptr_;
		}
		inline static Root* InsPtr()
		{
			if (!ins_ptr_)	ins_ptr_ = new Root();
			return ins_ptr_;
		}
		
		inline static void DestroyIns()
		{
			if (ins_ptr_)
			{
				delete ins_ptr_;
				ins_ptr_ = NULL;
			}
		}
		
	private:
		Renderer*		renderer_;
		SceneMgr*		scene_mgr_;
		InputMgr*		input_mgr_;
		TextureMgr*		texture_mgr_;
		FontMgr*		font_mgr_;
		ShaderMgr*		shader_mgr_;

		void*			window_handle_;
		
		static Root* ins_ptr_;
	};
}

#endif // ERI_ROOT_H
