/*
 *  skeleton_actor.h
 *  eri
 *
 *  Created by exe on 2010/11/3.
 *  Copyright 2010 cobbler. All rights reserved.
 *
 */

#ifndef ERI_SKELETON_ACTOR
#define ERI_SKELETON_ACTOR

#include "scene_actor.h"
#include "shared_skeleton.h"

namespace ERI
{
	
	struct AnimSetting
	{
		AnimSetting(int _idx = -1,
					float _speed_rate = 1.0f,
					bool _is_loop = true,
					bool _is_blend_begin = true,
					bool _is_inverse = false)
			:
			idx(_idx),
			speed_rate(_speed_rate),
			is_loop(_is_loop),
			is_blend_begin(_is_blend_begin),
			is_inverse(_is_inverse)
		{
		}

		bool operator==(const AnimSetting& setting) const
		{
			return (idx == setting.idx
					&& speed_rate == setting.speed_rate
					&& is_loop == setting.is_loop
					&& is_blend_begin == setting.is_blend_begin
					&& is_inverse == setting.is_inverse);
		}
		
		int		idx;
		float	speed_rate;
		bool	is_loop, is_blend_begin, is_inverse;
	};
	
	class SkeletonNodeIns
	{
	public:
		SkeletonNodeIns();
		
		// TODO: should put to another anim class interface ...
		
		void SetTime(float current_time, const AnimSetting& setting);
		
		//
		
		PoseSample*		attached_sample;
		Transform		local_pose;
		Matrix4			global_matrix;
		Matrix4			matrix_palette;
		
	private:
		
		// TODO: should put to another anim class interface ...
		
		void UpdateKey(float current_time, const AnimSetting& setting);
		void UpdateLocalPose(float current_time);

		int				current_start_key_, next_start_key_;
	};
	
	class SkeletonIns
	{
	public:
		SkeletonIns(const SharedSkeleton* resource_ref);
		~SkeletonIns();

		// TODO: should put to another anim class interface ...
		
		void SetAnim(const AnimSetting& setting);
		void SetTimePercent(float time_percent);
		float GetTimePercent() const;
		bool AddTime(float add_time);
		float GetTime() const;
		bool IsAnimEnd() const;
		void CancelLoop();
		
		//
		
		int FindNodeIdxByName(const std::string& name) const;
		const Matrix4& GetNodeCurrentTransform(int idx) const;
		int GetAnimClipNum() const;
		
		//
		
		int GetVertexBufferSize();
		int FillVertexBuffer(void* buffer);
		void GetVertexInfo(GLenum& vertex_type, VertexFormat& vertex_format);

		void UpdatePose();
		
	private:
		void AttachSample();
		
		const SharedSkeleton*	resource_ref_;

		std::vector<SkeletonNodeIns>	node_ins_array_;
		
		// TODO: should put to another anim class interface ...
		
		AnimSetting		anim_setting_;
		float			anim_duration_;
		float			anim_current_time_;
		float			pose_updated_time_;
	};
	
	class SkeletonActor : public SceneActor
	{
	public:
		SkeletonActor(const SharedSkeleton* resource_ref);
		virtual ~SkeletonActor();
		
		void ChangeResource(const SharedSkeleton* resource_ref);
		
		void Update(float delta_time);
		
		void SetAnim(const AnimSetting& setting);
		void SetTimePercent(float time_percent);
		
		void PlayAnim(const AnimSetting& setting,
					  bool wait_current_finish = true,
					  bool recover_current_loop = true);
		
		int	GetAnimIdx();
		
		inline const SkeletonIns* skeleton_ins() { return skeleton_ins_; }
		
	private:
		void UpdateVertexBuffer();
		
		SkeletonIns*	skeleton_ins_;
		
		void*			vertex_buffer_;
		int				vertex_buffer_size_;

		AnimSetting		curr_anim_, next_anim_;
		bool			recover_loop_;
	};
	
}

#endif // ERI_SKELETON_ACTOR
