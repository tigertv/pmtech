#include "polyspoon_math.h"

/*=========================================================*\
|	deg_to_rad - converts degree angles s32o radians
\*=========================================================*/
f32 psmath::deg_to_rad(f32 degree_angle)
{
	return( degree_angle * _PI_OVER_180 );
}

/*=========================================================*\
|	rad_to_deg - converts radian angles s32o dagrees
\*=========================================================*/
f32 psmath::rad_to_deg(f32 radian_angle)
{
	return( radian_angle * _180_OVER_PI );
}

/*=========================================================*\
|	absolute - retruns always a positive f32
\*=========================================================*/
f32 psmath::absolute(f32 value)
{
	if(value < 0.0f) value *= - 1;

	return value;
}

/*=========================================================*\
|	absolute_smallest_of - finds the smallest value of 2 params
\*=========================================================*/
f32 psmath::absolute_smallest_of(f32 value_1,f32 value_2)
{
	if(absolute(value_1) < absolute(value_2)) return value_1;
	else return value_2;
}

/*=========================================================*\
|	cross - retruns a Vector3f cross product result
\*=========================================================*/
vec3f psmath::cross(vec3f v1, vec3f v2)
{
	vec3f result;

	result.x = ((v1.y * v2.z) - (v1.z * v2.y));
	result.y = ((v1.x * v2.z) - (v1.z * v2.x));
	result.z = ((v1.x * v2.y) - (v1.y * v2.x));

	result.y *= -1;

	return result;
}

/*=========================================================*\
|	cross - retruns a vector cross prod result
\*=========================================================*/
f32 psmath::cross( vec2f v1, vec2f v2 )
{
	return v1.x * v2.y - v1.y * v2.x;
}

/*=========================================================*\
|	dot - retruns a f32 dot product result
\*=========================================================*/
f32 psmath::dot(vec3f v1,vec3f v2)
{
	return ((v1.x * v2.x) + (v1.y * v2.y) + (v1.z * v2.z));
}

/*=========================================================*\
|	perp - essentially 2d cross product to get the normal
|		 - of a 2d line well not the normal, but thats what
|		 - it looks like.
\*==========================================================*/
vec2f psmath::perp(vec2f v1, s32 hand)
{
	switch(hand)
	{
		case LEFT_HAND:
		{
			return vec2f(v1.y,-v1.x);
		}
		break;

		case RIGHT_HAND:
		{
			return vec2f(-v1.y,v1.x);
		}
		break;
	}

	//return left hand by default
	return vec2f(v1.y,-v1.x); 
}

/*=========================================================*\
|	magnitude - retruns a f32 the length of a vector
\*=========================================================*/
f32 psmath::magnitude(vec3f v)
{
	return (f32) sqrt((v.x * v.x) + (v.y * v.y) + (v.z * v.z));
}

f32 psmath::magnitude(vec2f v)
{
	return (f32) sqrt((v.x * v.x) + (v.y * v.y));
}

/*=========================================================*\
|	distance - retruns a f32 the distance between 2 pos32s
\*=========================================================*/
f32 psmath::distance(vec3f p1, vec3f p2)
{
	double d = sqrt( (p2.x - p1.x) * (p2.x - p1.x) + 
					 (p2.y - p1.y) * (p2.y - p1.y) + 
					 (p2.z - p1.z) * (p2.z - p1.z));

	return (f32) d;
}

f32 psmath::distanceSq(vec2f p1, vec2f p2)
{
	f32 d =  (p2.x - p1.x) * (p2.x - p1.x) + (p2.y - p1.y) * (p2.y - p1.y);
	return  d;
}

/*=========================================================*\
|	normalise - scale a vector to unit length
\*=========================================================*/
vec3f psmath::normalise(vec3f v){

	f32 r_mag = 1.0f / magnitude(v);

	v.x *= r_mag;
	v.y *= r_mag;
	v.z *= r_mag;
	
	return v;
}

vec2f psmath::normalise(vec2f v){

	f32 mag = 1.0f / magnitude(v);

	v *= mag;	

	return v;
}

vec3f psmath::project(vec3f v, mat4 view, mat4 proj, vec2i viewport, bool normalise_coordinates)
{
	mat4 res = proj * view;

	f32 depth = 1;

	vec3f screen_space_coord = res.homogeneous_multiply(v,&depth);
	screen_space_coord.x /= depth;
	screen_space_coord.y /= depth;
	screen_space_coord.z /= depth;

	screen_space_coord = (screen_space_coord * 0.5f);
	screen_space_coord.x += 0.5f;
	screen_space_coord.y += 0.5f;
	screen_space_coord.z += 0.5f;

	if(!normalise_coordinates)
	{
		screen_space_coord.x *= viewport.x;
		screen_space_coord.y *= viewport.y;
	}

	return screen_space_coord;
}

vec3f psmath::unproject( vec3f screen_space_pos, mat4 view, mat4 proj, vec2i viewport )
{
	//inverse the view projection matrix
	mat4 view_proj = proj * view;
	mat4 inverse_view_projection = view_proj.inverse4x4();

	//normalise the screen coordinates
	vec3f normalised_device_coordinates;
	normalised_device_coordinates.x = (((screen_space_pos.x / viewport.x) * 2.0f) - 1.0f);
	normalised_device_coordinates.y = (((screen_space_pos.y / viewport.y) * 2.0f) - 1.0f);
	
	normalised_device_coordinates.z = screen_space_pos.z; //((screen_space_pos.z * 2.0f) - 1.0f);

	//multiply the normalised device coordinates by the inverse view projection matrix
	f32 w = 1.0f;
	vec3f post_perspective_coordinates = inverse_view_projection.homogeneous_multiply(normalised_device_coordinates, &w);
	w = 1.0f / w;

	//perform the inverse of perspective divide
	vec3f world_space_coordinates;
	world_space_coordinates.x = post_perspective_coordinates.x * w;
	world_space_coordinates.y = post_perspective_coordinates.y * w;
	world_space_coordinates.z = post_perspective_coordinates.z * w;

	return world_space_coordinates;
}

#if 0

/*=========================================================*\
|	billboard_cylindrical_begin
|			- modifies x,z axis to always face the camera
\*=========================================================*/
void psmath::billboard_cylindrical_begin()
{
#if 0
	f32 modelview[16];
	s32 i,j;

	// save the current modelview matrix
	glPushMatrix();

	// get the current modelview matrix
	glGetf32v(GL_MODELVIEW_MATRIX , modelview);

	//undo all rotations
	for( i=0; i<3; i+=2 ) 
	{
	    for( j=0; j<3; j++ ) 
		{
			if ( i==j )
				 modelview[i*4+j] = 1.0;
			else
				modelview[i*4+j] = 0.0;
	    }
	}

	// set the modelview matrix
	glLoadMatrixf(modelview);
#endif

	PEN_ERR;
}

/*=========================================================*\
|	billboard_spherical_begin
|		- modifies x,y and z axis to always face the camera
\*=========================================================*/
void psmath::billboard_spherical_begin() 
{
	PEN_ERR;

#if 0
	f32 modelview[16];
	s32 i,j;

	// save the current modelview matrix
	glPushMatrix();

	// get the current modelview matrix
	glGetf32v(GL_MODELVIEW_MATRIX , modelview);

	// undo all rotations
	for( i=0; i<3; i++ ) 
	{
	    for( j=0; j<3; j++ ) 
		{
			if ( i==j )
				modelview[i*4+j] = 1.0;
			else
				 modelview[i*4+j] = 0.0;
	    }
	}

	// set the modelview with no rotations
	glLoadMatrixf(modelview);
#endif
}

/*=========================================================*\
|	billboard_end - pop matrix wrapper for readability
\*=========================================================*/
void psmath::billboard_end()
{ 
#if 0
	glPopMatrix(); 
#endif

	PEN_ERR;
}

/*=========================================================*\
|	get normal - gets the normal vector of a polygon
|	NB * the normal is inverted in here, was coming out wrong
|		 in collision tests, keep an eye on this
\*=========================================================*/
vec3f psmath::get_normal(TRIANGLE t1)
{
	vec3f v1 = t1.m_vertices[2] - t1.m_vertices[0];
	vec3f v2 = t1.m_vertices[1] - t1.m_vertices[0];

	vec3f normal = cross(v1, v2);

	normal = normalise(normal) *= -1;

	return normal;		
}

vec3f psmath::get_normal(vec3f v1, vec3f v2, vec3f v3)
{
	vec3f vA = v3 - v1;
	vec3f vB = v2 - v1;

	vec3f normal = cross(vA, vB);

	//negate for opengl handedness
	normal = normalise(normal) *= -1;

	return normal;		
}

/*=========================================================*\
|	classify_sphere - classify a sphere to a plane
|					- based on simple distance calculation
\*=========================================================*/
s32 psmath::classify_sphere(SPHERE s1, vec3f p, vec3f normal, f32 *distance)
{
	f32 d = plane_distance(normal, p);

	*distance = (normal.x * s1.m_position.x + normal.y * s1.m_position.y + normal.z * s1.m_position.z + d);

	//if the distance is less than the radius and s32ersection occurs
	if(absolute(*distance) < s1.m_radius)
	{	
		return s32ERSECTS;
	}
	else if(*distance >= s1.m_radius)
	{	
		return INFRONT;
	}
	
	//else the sphere is behind the plane
	return BEHIND;

}

s32 psmath::classify_sphere( SPHERE s1, PLANE p1 )
{
	f32 plane_d = plane_distance(p1.m_normal, p1.m_pos32_on_plane);

	f32 d = (dot(p1.m_normal, s1.m_position) + plane_d);

	//if the distance is less than the radius and s32ersection occurs
	if(absolute(d) < s1.m_radius)
	{	
		return s32ERSECTS;
	}
	else if(d >= s1.m_radius)
	{	
		return INFRONT;
	}

	//else the sphere is behind the plane
	return BEHIND;
}
/*=========================================================*\
|	plane_distance - find the distance from plane to a pos32
\*=========================================================*/
f32 psmath::plane_distance(vec3f normal, vec3f pos32)
{
	f32 distance = 0;

	//Use the plane equation to find the distance D
	//negative dot product between normal vector and pos32 (p)
	distance = dot(normal,pos32) * -1; 

	return distance;	
}

/*=========================================================*\
|	angle_between_vectors 
|			- find the angle (radians) between 2 Vectors
\*=========================================================*/
f32 psmath::angle_between_vectors(vec3f v1,vec3f v2)
{
	//get the dot product
	f32 dp = dot(v1,v2);

	//get magnitude product
	double mp = magnitude(v1) * magnitude(v2);

	//get the angle between the 2 above (radians)
	f32 angle = (f32) acos( dp / mp );

	//ensure angle is a number
	if(_isnan(angle))
	{
		return 0.0f;
	}

	//return the angle in radians
	return angle;
}

/*=========================================================*\
|	closest_pos32_on_line 
|		- find the closest pos32 on a line to another pos32
\*=========================================================*/
vec3f psmath::closest_point_on_line(vec3f l1, vec3f l2, vec3f p, bool clamp )
{
	//create a vector from line start to the pos32.
	vec3f v1 = p - l1;

	//get the normalised direction vector of the line
    vec3f v2 = normalise(l2 - l1);

	//use the distance formula to get the length of the line
    f32 d = distance(l1, l2);

	//using dot product project v1 onto v2
    f32 t = dot(v2, v1);

	if( clamp )
	{
		//if the pos32s at either end of the line
		//the pos32 is before the line start
		if (t <= 0) 
		{
			return l1;
		}

		//the pos32 is after the line end
		if (t >= d) 
		{
			return l2;
		}
	}

	//otherwise the pos32 is on the line

	//vector of length t and direction of the line
    vec3f v3 = v2 * t;

	//to get the closest pos32 simply add v3 to the starting pos32 of the line
    vec3f closest_pos32 = l1 + v3;

	return closest_pos32;
}

f32 psmath::distance_on_line(vec3f l1, vec3f l2, vec3f p, bool clamp )
{
	//create a vector from line start to the pos32.
	vec3f v1 = p - l1;

	//get the normalised direction vector of the line
	vec3f v2 = normalise(l2 - l1);

	//use the distance formula to get the length of the line
	f32 d = distance(l1, l2);

	//using dot product project v1 onto v2
	f32 t = dot(v2, v1);

	t /= d;

	if( clamp )
	{
		//if the pos32s at either end of the line
		//the pos32 is before the line start
		if (t <= 0 ) 
		{
			return 0.0f;
		}

		//the pos32 is after the line end
		if (t >= 1.0f ) 
		{
			return 1.0f;
		}
	}

	return t;
}

/*=========================================================*\
|	closest_pos32_on_AABB3D 
|		- find the closest pos32 on an AABB to another pos32
\*=========================================================*/
vec3f psmath::closest_point_on_AABB3D(AABB3D b1, vec3f p)
{
	//this could be optimised out
	vec3f b1_extent_min = b1.m_position - b1.m_size;
	vec3f b1_extent_max = b1.m_position + b1.m_size;

	if(p.x < b1_extent_min.x) p.x = b1_extent_min.x;
	else if(p.x > b1_extent_max.x) p.x = b1_extent_max.x;

	if(p.y < b1_extent_min.y) p.y = b1_extent_min.y;
	else if(p.y > b1_extent_max.y) p.y = b1_extent_max.y;

	if(p.z < b1_extent_min.z) p.z = b1_extent_min.z;
	else if(p.z > b1_extent_max.z) p.z = b1_extent_max.z;

	return p;
}

/*=========================================================*\
|	get_axes_from_OBB
|		- finds the 3 oriented axes of an OBB 
\*=========================================================*/
void psmath::get_axes_from_OBB(OBB3D b1, vec3f *axes)
{
	//make some vertices for each axis
	vec3f x_axis[2];
	vec3f y_axis[2];
	vec3f z_axis[2];

	x_axis[0] = vec3f(-1,1,1);
	x_axis[1] = vec3f( 1,1,1);

	y_axis[0] = vec3f( 1,-1,1);
	y_axis[1] = vec3f( 1, 1,1);

	z_axis[0] = vec3f( 1,1,-1);
	z_axis[1] = vec3f( 1,1, 1);

	//orient the verts to the obb orientation
	for(s32 i = 0; i < 2; i++)
	{
		x_axis[i] = b1.m_orientation_matrix * x_axis[i];
		y_axis[i] = b1.m_orientation_matrix * y_axis[i];
		z_axis[i] = b1.m_orientation_matrix * z_axis[i];
	}

	//now take a vector between each axis verts to get the final
	//normalised axis
	axes[0] = normalise(x_axis[1] - x_axis[0]);
	axes[1] = normalise(y_axis[1] - y_axis[0]);
	axes[2] = normalise(z_axis[1] - z_axis[0]);
}

/*=========================================================*\
|	find_extents
|		- finds the min and max extents of a group of vertices
|		- on a given axis (return by pos32ers min and max)
|		- extent is returned as a f32 value from the
|		- dot product (the distance along an axis)
\*=========================================================*/
void psmath::find_extents(vec3f axis, vec3f *vertices, unsigned s32 vertex_count, f32 *min, f32 *max)
{
	f32 *projections = new f32[vertex_count];

	for(unsigned s32 i = 0; i < vertex_count; i++)
	{
		projections[i] = dot(axis,vertices[i]);
	}

	*min = projections[0];
	*max = projections[0];

	for(unsigned s32 i = 0; i < vertex_count; i++)
	{
		if(projections[i] < *min) *min = projections[i];
		else if(projections[i] > *max) *max = projections[i];
	}

	delete projections;
}

void psmath::find_extents(vec3f axis, Vector3fArray vertices, f32 *min, f32 *max)
{
	f32 *projections = new f32[vertices.size()];

	for(unsigned s32 i = 0; i < vertices.size(); i++)
	{
		projections[i] = dot(axis,vertices[i]);
	}

	*min = projections[0];
	*max = projections[0];

	for(unsigned s32 i = 0; i < vertices.size(); i++)
	{
		if(projections[i] < *min) *min = projections[i];
		else if(projections[i] > *max) *max = projections[i];
	}

	delete projections;
}

/*=========================================================*\
|	find_extents
|		- finds the min and max extents of a group of vertices
|		- on a given axis (return by pos32ers min and max)
|		- extent is returned as a vector3f value of the position
|		- the extent is on the axis
\*=========================================================*/
void psmath::find_extents(vec3f axis, vec3f *vertices, unsigned s32 vertex_count, vec3f *min_position, vec3f *max_position)
{
	f32 min, max;

	find_extents(axis,vertices,vertex_count,&min,&max);

	*min_position = axis * min;
	*max_position = axis * max;
}

vec3f psmath::RAY_vs_PLANE( RAY_3D ray, PLANE plane )
{
	//this is wrong i think
	vec3f v = ray.m_direction_vector;
	v.normalise();

	vec3f p = ray.m_pos32_on_ray;
	vec3f n = plane.m_normal;

	f32 d = plane_distance(n, plane.m_pos32_on_plane);
	f32 t = -(dot(p,n) + d) / dot(v,n);

	vec3f pos32_on_plane = p + (v * t);

	return pos32_on_plane;
}

void psmath::compute_tangents( vec3f v1, vec3f v2, vec3f v3, vec2f t1, vec2f t2, vec2f t3, vec3f *tangent, vec3f *bitangent, bool normalise )
{
	//calculate vectors of the edges of the triangle
	vec3f vA = v2 - v1;
	vec3f vB = v3 - v1;

	vec2f vA_uv = t2 - t1;
	vec2f vB_uv = t3 - t1;

	//get the determinant of the uv-coords 2x2 matrix
	f32 det = vA_uv.y * vB_uv.x - vA_uv.x * vB_uv.y;

	if ( det != 0.0f ) 
	{
		f32 one_over_det = 1.0f / det;

		*tangent   = ((vA * -vB_uv.y) + (vB * vA_uv.y)) * one_over_det;
		*bitangent = ((vA * -vB_uv.x) + (vB * vA_uv.x)) * one_over_det;

		if(normalise)
		{
			tangent->normalise();
			bitangent->normalise();
		}
	}
}

bool psmath::pos32_inside_triangle( vec3f v1, vec3f v2, vec3f v3, vec3f p )
{
	vec3f cp1, cp2;

	//edge 1
	cp1 = cross(v2 - v1, v3 - v1);
	cp2 = cross(v2 - v1, p - v1);
	if(dot(cp1, cp2) < 0) return false;

	//edge 2
	cp1 = cross(v3 - v1, v2 - v1);
	cp2 = cross(v3 - v1, p - v1);
	if(dot(cp1, cp2) < 0) return false;

	//edge 3
	cp1 = cross(v3 - v2, v1 - v2);
	cp2 = cross(v3 - v2, p - v2);
	if(dot(cp1, cp2) < 0) return false;

	return true;
}

#endif