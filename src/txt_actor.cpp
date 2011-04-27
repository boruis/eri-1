//
//  txt_actor.cpp
//  eri
//
//  Created by exe on 4/24/11.
//  Copyright 2011 cobbler. All rights reserved.
//

#include "txt_actor.h"

#include "root.h"
#include "font_mgr.h"

#include "platform_helper.h"

namespace ERI
{

int CreateUnicodeArray(const std::string& txt, bool is_utf8, uint32_t*& out_chars)
{
  int length = 0;
  
  if (is_utf8)
  {
    int max_buff_size = txt.length() * 2;
    out_chars = new uint32_t[max_buff_size];
    length = GetUnicodeFromUTF8(txt, max_buff_size, out_chars);
  }
  else
  {
    out_chars = new uint32_t[txt.length()];
    
    for (int i = 0; i < txt.length(); ++i)
      out_chars[i] = txt[i];
    
    length = txt.length();
  }
  
  return length;
}

#pragma mark TxtMeshConstructor
  
class TxtMeshConstructor
{
 public:
  TxtMeshConstructor(TxtActor* owner)
    : owner_(owner),
      vertices_(NULL)
  {
    ASSERT(owner_);
  }
  
  virtual ~TxtMeshConstructor()
  {
    if (vertices_) free(vertices_);
  }

  virtual void Construct() = 0;
  
 protected:
  TxtActor* owner_;
  vertex_2_pos_tex* vertices_;
};
  
#pragma mark SpriteTxtMeshConstructor

class SpriteTxtMeshConstructor : public TxtMeshConstructor
{
 public:
  SpriteTxtMeshConstructor(TxtActor* owner)
    : TxtMeshConstructor(owner)
  {
    char tex_name[32];
    sprintf(tex_name, "txt:%p", owner_);
    tex_name_ = tex_name;
  }
  
  virtual ~SpriteTxtMeshConstructor()
  {
    owner_->SetMaterial(NULL);
    Root::Ins().texture_mgr()->ReleaseTexture(tex_name_);
  }
  
  virtual void Construct()
  {
    bool first_construct = owner_->material_data_.used_unit == 0;
    
    if (!first_construct)
    {
      Root::Ins().texture_mgr()->ReleaseTexture(tex_name_);
    }
    
    uint32_t* chars;
    int length = CreateUnicodeArray(owner_->txt_, owner_->is_utf8_, chars);

    int width, height;
    const Texture* tex = owner_->font_ref_->CreateSpriteTxt(tex_name_,
                                                            chars,
                                                            length,
                                                            owner_->is_anti_alias_,
                                                            width,
                                                            height);
    
    delete [] chars;
    
    owner_->SetMaterial(tex);
    owner_->width_ = width;
    owner_->height_ = height;
    
    if (first_construct)
      owner_->SetTextureFilter(owner_->font_ref_->filter_min(),
                               owner_->font_ref_->filter_min());
    
    if (owner_->render_data_.vertex_buffer == 0)
		{
			glGenBuffers(1, &owner_->render_data_.vertex_buffer);
		}
		    
    float size_scale = static_cast<float>(owner_->font_size_) / owner_->font_ref_->size();

    Vector2 size(width * size_scale, height * size_scale);
    Vector2 start;
    if (owner_->is_pos_center_)
    {
      start.x = -size.x * 0.5f;
      start.y = size.y * 0.5f;
    }
    
    Vector2 uv_size(owner_->width_ / tex->width, owner_->height_ / tex->height);
    
    //  2 - 3
    //  | \ |
    //  0 - 1
    
    vertex_2_pos_tex v[] = {
      { start.x,          start.y - size.y, 0.0f,       uv_size.y },
      { start.x + size.x, start.y - size.y, uv_size.x,  uv_size.y },
      { start.x,          start.y         , 0.0f,       0.0f      },
      { start.x + size.x, start.y         , uv_size.x,  0.0f      }
    };

    owner_->render_data_.vertex_type = GL_TRIANGLE_STRIP;
    owner_->render_data_.vertex_format = POS_TEX_2;
    owner_->render_data_.vertex_count = 4;
    
    glBindBuffer(GL_ARRAY_BUFFER, owner_->render_data_.vertex_buffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(v), v, GL_STATIC_DRAW);
  }
  
 private:
  std::string tex_name_;
};
  
#pragma mark AtlasTxtMeshConstructor
  
class AtlasTxtMeshConstructor : public TxtMeshConstructor
{
 public:
  AtlasTxtMeshConstructor(TxtActor* owner)
    : TxtMeshConstructor(owner),
      now_len_max_(0),
      now_len_(0)
  {
  }
  
  virtual ~AtlasTxtMeshConstructor()
  {
  }
  
  virtual void Construct()
  {
    uint32_t* chars;
    now_len_ = CreateUnicodeArray(owner_->txt_, owner_->is_utf8_, chars);
    
    int unit_vertex_num = 6;
    
    if (now_len_max_ < now_len_)
    {
      now_len_max_ = now_len_;
      
      if (vertices_) free(vertices_);
      vertices_ = static_cast<vertex_2_pos_tex*>(malloc(now_len_max_ * unit_vertex_num * sizeof(vertex_2_pos_tex)));
      
      if (owner_->render_data_.vertex_buffer != 0)
      {
        glDeleteBuffers(1, &owner_->render_data_.vertex_buffer);
      }
      
      glGenBuffers(1, &owner_->render_data_.vertex_buffer);
    }
    
    float inv_tex_width = 1.0f / owner_->font_ref_->texture()->width;
    float inv_tex_height = 1.0f / owner_->font_ref_->texture()->height;
    float size_scale = static_cast<float>(owner_->font_size_) / owner_->font_ref_->size();
    
    TxtActor::CalculateSize(chars,
                            now_len_,
                            owner_->font_ref_,
                            owner_->font_size_,
                            owner_->width_,
                            owner_->height_);
    
    float start_x = owner_->is_pos_center_ ? owner_->width_ * -0.5f : 0;
    float start_y = owner_->is_pos_center_ ? owner_->height_ * 0.5f : 0;
    int start_idx = 0;
    float scroll_u, scroll_v, unit_u, unit_v, offset_x, offset_y, size_x, size_y;
    
    int invisible_num = 0;
    
    vertex_2_pos_tex* v;
    
    for (int i = 0; i < now_len_; ++i)
    {
      if (chars[i] == '\n')
      {
        start_x = owner_->is_pos_center_ ? owner_->width_ * -0.5f : 0;
        start_y -= owner_->font_ref_->common_line_height() * size_scale;
        ++invisible_num;
      }
      else
      {
        const CharSetting& setting = owner_->font_ref_->GetCharSetting(chars[i]);
        
        scroll_u = setting.x * inv_tex_width;
        scroll_v = setting.y * inv_tex_height;
        unit_u = (setting.width - 1) * inv_tex_width;
        unit_v = (setting.height - 1) * inv_tex_height;
        
        offset_x = setting.x_offset * size_scale;
        offset_y = setting.y_offset * size_scale;
        size_x = (setting.width - 1) * size_scale;
        size_y = (setting.height - 1) * size_scale;
        
        v = &vertices_[start_idx];
        
        // 2,3 -  5
        //  |  \  |
        //  0 -  1,4
        
        v[0].position[0] = start_x + offset_x;
        v[0].position[1] = start_y - offset_y - size_y;
        v[0].tex_coord[0] = scroll_u;
        v[0].tex_coord[1] = scroll_v + unit_v;
        
        v[1].position[0] = start_x + offset_x + size_x;
        v[1].position[1] = start_y - offset_y - size_y;
        v[1].tex_coord[0] = scroll_u + unit_u;
        v[1].tex_coord[1] = scroll_v + unit_v;
        
        v[2].position[0] = start_x + offset_x;
        v[2].position[1] = start_y - offset_y;
        v[2].tex_coord[0] = scroll_u;
        v[2].tex_coord[1] = scroll_v;
        
        v[3].position[0] = start_x + offset_x;
        v[3].position[1] = start_y - offset_y;
        v[3].tex_coord[0] = scroll_u;
        v[3].tex_coord[1] = scroll_v;
        
        v[4].position[0] = start_x + offset_x + size_x;
        v[4].position[1] = start_y - offset_y - size_y;
        v[4].tex_coord[0] = scroll_u + unit_u;
        v[4].tex_coord[1] = scroll_v + unit_v;
        
        v[5].position[0] = start_x + offset_x + size_x;
        v[5].position[1] = start_y - offset_y;
        v[5].tex_coord[0] = scroll_u + unit_u;
        v[5].tex_coord[1] = scroll_v;
        
        start_x += setting.x_advance * size_scale;
        start_idx += unit_vertex_num;
      }
    }
    
    delete [] chars;
    
    owner_->render_data_.vertex_count = (now_len_ - invisible_num) * unit_vertex_num;
    owner_->render_data_.vertex_type = GL_TRIANGLES;
    
    glBindBuffer(GL_ARRAY_BUFFER, owner_->render_data_.vertex_buffer);
    glBufferData(GL_ARRAY_BUFFER,
                 owner_->render_data_.vertex_count * sizeof(vertex_2_pos_tex),
                 vertices_,
                 GL_DYNAMIC_DRAW);
  }
  
 private:
  int now_len_max_;
  int now_len_;
};

#pragma mark TxtActor
  
TxtActor::TxtActor(const std::string& font_path, int font_size,
                   bool is_pos_center /*= false*/,
                   bool is_utf8 /*= false*/,
                   bool is_anti_alias /*= true*/)
  : font_ref_(NULL),
    font_size_(font_size),
    is_pos_center_(is_pos_center),
    is_utf8_(is_utf8),
    is_anti_alias_(is_anti_alias),
    width_(0.0f),
    height_(0.0f),
    area_border_(0.0f)
{
  font_ref_ = Root::Ins().font_mgr()->GetFont(font_path);
  
  ASSERT(font_ref_);
  
  if (font_ref_->is_atlas())
  {
    mesh_constructor_ = new AtlasTxtMeshConstructor(this);
    
    SetMaterial(font_ref_->texture(),
                font_ref_->filter_min(),
                font_ref_->filter_mag());
  }
  else
  {
    mesh_constructor_ = new SpriteTxtMeshConstructor(this);
  }
}

TxtActor::~TxtActor()
{
  delete mesh_constructor_;
}

void TxtActor::SetTxt(const std::string& txt)
{
  txt_ = txt;
  
  mesh_constructor_->Construct();
}

void TxtActor::CalculateSize(const std::string& txt,
                             const Font* font,
                             int font_size,
                             float& width,
                             float& height,
                             bool is_utf8 /*= false*/)
{
  ASSERT(font);
  
  uint32_t* chars;
  int length = CreateUnicodeArray(txt, is_utf8, chars);
  
  CalculateSize(chars, length, font, font_size, width, height);
  
  delete [] chars;
}

void TxtActor::CalculateSize(const uint32_t* chars,
                             int length,
                             const Font* font,
                             int font_size,
                             float& width,
                             float& height)
{
  ASSERT(font);
  
  width = 0.0f;
  
  float size_scale = static_cast<float>(font_size) / font->size();  
  height = font->common_line_height() * size_scale;
  
  if (length == 0)
    return;

  float now_width = 0;
  for (int i = 0; i < length; ++i)
  {
    if (chars[i] == '\n')
    {
      if (now_width > width) width = now_width;
      now_width = 0;
      height += font->common_line_height() * size_scale;
    }
    else
    {
      const CharSetting& setting = font->GetCharSetting(chars[i]);
      now_width += setting.x_advance * size_scale;
    }
  }
  
  if (now_width > width) width = now_width;
}

bool TxtActor::IsInArea(const Vector3& local_space_pos)
{
  if (local_space_pos.x >= ((is_pos_center_ ? (-width_ / 2) : 0) - area_border_)
			&& local_space_pos.x <= ((is_pos_center_ ? (width_ / 2) : width_) + area_border_)
			&& local_space_pos.y >= ((is_pos_center_ ? (-height_ / 2) : -height_) - area_border_)
			&& local_space_pos.y <= ((is_pos_center_ ? (height_ / 2) : 0) + area_border_))
  {
    return true;
  }
  
  return false;
}
  
}