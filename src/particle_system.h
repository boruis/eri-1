/*
 *  particle_system.h
 *  eri
 *
 *  Created by seedstudio03 on 2010/10/12.
 *  Copyright 2010 cobbler. All rights reserved.
 *
 */

#ifndef ERI_PARTICLE_SYSTEM_H
#define ERI_PARTICLE_SYSTEM_H

#include "scene_actor.h"

namespace ERI
{
	
#pragma mark Particle
	
	struct Particle
	{
		Particle() { Reset(); }
		
		void Reset()
		{
			in_use = false;
			scale = Vector2(1.0f, 1.0f);
		}
		
		Vector2	pos;
		Vector2	velocity;
		Vector2	size;
		Vector2 scale;
		float	rotate_angle;
		float	rotate_speed;
		Color	color;
		int		color_interval;
		
		// life
		
		float	life;
		float	lived_time;
		float	lived_percent;
		bool	in_use;
		
		std::vector<float> affector_timers;
	};
	
#pragma mark Emitter
	
	enum EmitterType
	{
		EMITTER_BOX,
		EMITTER_END
	};
	
	class BaseEmitter
	{
	public:
		BaseEmitter(float rate, float angle_min, float angle_max);
		virtual ~BaseEmitter();
		
		bool CheckIsTimeToEmit(float delta_time, int& out_emit_num);
		
		float GetEmitAngle();
		
		virtual void GetEmitPos(Vector2& pos) = 0;
		
		virtual BaseEmitter* Clone() = 0;
		
		inline float rate() { return rate_; }
		inline float angle_min() { return angle_min_; }
		inline float angle_max() { return angle_max_; }
	
	private:
		float	rate_;
		float	angle_min_, angle_max_;
		
		float	emit_interval_;
		float	emit_remain_time_;
	};
	
	class BoxEmitter : public BaseEmitter
	{
	public:
		BoxEmitter(Vector2 half_size, float rate, float angle_min, float angle_max);
		virtual ~BoxEmitter();
		
		virtual BaseEmitter* Clone();
		
	protected:
		virtual void GetEmitPos(Vector2& pos);
		
	private:
		Vector2	half_size_;
	};
	
	class CircleEmitter : public BaseEmitter
	{
	public:
		CircleEmitter(float radius, float rate, float angle_min, float angle_max);
		virtual ~CircleEmitter();
		
		virtual BaseEmitter* Clone();
		
	protected:
		virtual void GetEmitPos(Vector2& pos);
		
	private:
		float	radius_;
	};
	
#pragma mark Affector
	
	class BaseAffector
	{
	public:
		BaseAffector() : period_(-1.f) {}
		virtual ~BaseAffector() {}
		
		virtual void InitSetup(Particle* p) {}
		virtual void Update(float delta_time, Particle* p) = 0;
		
		virtual BaseAffector* Clone() = 0;
		
		inline float period() { return period_; }
		inline void set_period(float period) { period_ = period; }
		
	private:
		float period_;
	};
	
	class RotateAffector : public BaseAffector
	{
	public:
		RotateAffector(float speed, float acceleration = 0.0f);
		virtual ~RotateAffector();

		virtual void InitSetup(Particle* p);
		virtual void Update(float delta_time, Particle* p);
		virtual BaseAffector* Clone();
		
	private:
		float	speed_, acceleration_;
	};
	
	class ForceAffector : public BaseAffector
	{
	public:
		ForceAffector(const Vector2& acceleration);
		virtual ~ForceAffector();
		
		virtual void Update(float delta_time, Particle* p);
		virtual BaseAffector* Clone();
		
	private:
		Vector2	acceleration_;
	};
	
	class AccelerationAffector : public BaseAffector
	{
	public:
		AccelerationAffector(float acceleration);
		virtual ~AccelerationAffector();
		
		virtual void Update(float delta_time, Particle* p);
		virtual BaseAffector* Clone();
		
	private:
		float	acceleration_;
	};
	
	class ScaleAffector : public BaseAffector
	{
	public:
		ScaleAffector(const Vector2& speed);
		virtual ~ScaleAffector();
		
		virtual void Update(float delta_time, Particle* p);
		virtual BaseAffector* Clone();
		
	private:
		Vector2	speed_;
	};
	
	class ColorAffector : public BaseAffector
	{
	public:
		ColorAffector(const Color& start, const Color& end);
		virtual ~ColorAffector();
		
		virtual void InitSetup(Particle* p);
		virtual void Update(float delta_time, Particle* p);
		
		virtual BaseAffector* Clone() { return new ColorAffector(start_, end_); }
		
	private:
		Color	start_, end_;
	};
	
	class ColorIntervalAffector : public BaseAffector
	{
	public:
		ColorIntervalAffector();
		virtual ~ColorIntervalAffector();
		
		virtual void InitSetup(Particle* p);
		virtual void Update(float delta_time, Particle* p);
		
		virtual BaseAffector* Clone();
		
		void AddInterval(float lived_percent, const Color& color);
		
	private:
		struct ColorInterval
		{
			float	lived_percent;
			Color	color;
		};
		
		std::vector<ColorInterval>	intervals_;
	};
	
	
#pragma mark ParticleSystem
	
	struct ParticleSystemSetup
	{
		ParticleSystemSetup() :
			is_coord_relative(false),
			life(-1.0f),
			particle_size(Vector2(1.0f, 1.0f)),
			particle_life_min(1.0f), particle_life_max(1.0f),
			particle_speed_min(0.0f), particle_speed_max(0.0f),
			particle_rotate_min(0.0f), particle_rotate_max(0.0f),
			particle_scale_min(1.0f), particle_scale_max(1.0f)
		{
		}
		
		bool		is_coord_relative;
		
		float		life;
		
		Vector2		particle_size;
		
		float		particle_life_min, particle_life_max;
		float		particle_speed_min, particle_speed_max;
		float		particle_rotate_min, particle_rotate_max;
		float		particle_scale_min, particle_scale_max;
	};
	
	class ParticleSystem : public SceneActor
	{
	public:
		ParticleSystem(const ParticleSystemSetup* setup_ref);
		~ParticleSystem();
		
		void RefreshSetup();
		
		void SetEmitter(BaseEmitter* emitter);
		void AddAffector(BaseAffector* affector);
		
		void Play();
		bool IsPlaying();
		
		void Update(float delta_time);
		
		void ResetParticles();
		
		void SetTexAreaUV(float start_u, float start_v, float width, float height);
		
		inline const ParticleSystemSetup* setup_ref() { return setup_ref_; }
		
		inline void set_custom_life(float life) { custom_life_ = life; }
		inline float custom_life() { return custom_life_; }
		
	private:
		void EmitParticle(int num);
		Particle* ObtainParticle();
		
		void CreateBuffer();
		void UpdateBuffer();
		
		const ParticleSystemSetup*	setup_ref_;
		float						custom_life_;
		
		BaseEmitter*				emitter_;
		std::vector<BaseAffector*>	affectors_;
		
		std::vector<Particle*>		particles_;
		int							first_available_particle_idx_;
		
		vertex_2_pos_tex_color*		vertices_;
		unsigned short*				indices_;
		
		Vector2		system_scale_;
		Vector2		uv_start_, uv_size_;
		
		float		lived_time_;
	};
	
#pragma mark ParticleSystemCreator
	
	struct ParticleMaterialSetup
	{
		ParticleMaterialSetup() :
			tex_filter(FILTER_LINEAR),
			u_start(0.0f),
			v_start(0.0f),
			u_width(1.0f),
			v_height(1.0f),
			depth_write(true),
			blend_add(false)
		{
		}
		
		std::string					tex_path;
		TextureFilter				tex_filter;
		float						u_start, v_start, u_width, v_height;
		bool						depth_write;
		bool						blend_add;
	};
	
	struct ParticleSystemCreator
	{
		ParticleSystemCreator() : emitter(NULL) {}
		~ParticleSystemCreator();
		
		ParticleSystemSetup*		setup;
		BaseEmitter*				emitter;
		std::vector<BaseAffector*>	affectors;
		
		ParticleMaterialSetup		material_setup;
		
		ParticleSystem*	Create();
	};
	
#pragma mark script loader function
	
	ParticleSystemCreator* LoadParticleSystemCreatorByScriptFile(const std::string& path);

}

#endif // ERI_PARTICLE_SYSTEM_H