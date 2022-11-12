GAME ENGINE AND PLAYER MOVEMENT - HENRIQUE
Mouse movement:

Keyboard movement: I made each keyboard key add to the players acceleration by the direction
(m_forward, m_right, etc). Moreover, I made sure to cap the length of the acceleration vector
to ensure that you couldn't move faster diagonally.

Player physics: I made friction be equivalent to 1% of velocity at all times, and this is the first
thing that is subtracted from velocity, making the player gradually come to a stop. I made gravity
a constant, and only had it affect the player when flight mode is off. Once friction and gravity
are taken into consideration, acceleration is added to velocity.

Player collision: I chose to cast rays from the corners of my player's bounding box for this. For each
corner, I would cast rays in the three cardinal directions. This allows my player to slide along walls,
and means that I did not need separate collision checking for the ground. To allow my player to squeeze
through 1x1 spaces, I actually made the bounding box 0.1f smaller in all dimensions. I also check here
the size and direction of the y component of my casted rays, allowing me to determine if the player
is currently on the ground or not.

Placing and removing blocks: To place blocks, I casted a ray of length 3 in the direction of the camera's
look vector, and then gridmarched to determine which axis the ray was hitting. Depending on the axis, that
would determine when to place the block. For simplicity, only stone blocks can be placed. Similar logic was
used for removing blocks, but identifying the axis was not necessary and all I had to do was set the block
that was hit by my ray to zero.

PROCEDURAL TERRAIN GENERATION - BEN
Mountain Biome:
places blocks (from top to bottom) : (SNOW*), (WATER**), STONE
Grassland Biome:
places blocks (from top to bottom : (WATER**), GRASS, DIRT, STONE
 --- * Snow is only placed if height is greater than 200 
 --- ** Water is placed above heightMap up to 128 (if height map < 128)
 
 Mountain Biome Height Map: 
  - multi-hybrid fractal brownian motion (FBM) : basis noise function = abs(perlin)
    - H = .5
    - octaves = 8
    - perlin scale = 200
  -> 80 * smoothstep(FBM) + 100
  
 Grassland Biome Height Map:
 - multi hybrid fractal brownian motion (FBM) : basis noise function = 1 - abs(perlin)
    - H = .6
    - octaves = 8
    - perlin scale = 300
  -> 66 * smoothstep(FBM) + 111

Biome Interpolation Map:
- multi hybrid fractal brownian motion (FBM) : basis noise function = abs(perlin)
    - H = .9
    - octaves = 8
    - perlin scale = 300
 -> smoothStep(FBM)

Biome Interpolation:
- using the biome interpolation map as a weight (1 = fully mountain, 0 = fully grassland),
  LERP between height maps of each biome
- if weight > 0.5, populate coordinates with mountain blocks
    - else populate with grassland blocks
    
Challenges:
  - perlin noise showing 'tiles' based on scale factor, mitigated by smoothstep but still occurs occasionally
  - when using smoothstep, the min and max values returned by FBM vary greatly based on parameters, need to be optimized each time parameters change
  - many hours spent tweaking parameters to get desired results

Terrain Rendering and Chunking (Rafael):
