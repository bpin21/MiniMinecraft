#include "player.h"
#include <QString>
#include <iostream>

using namespace std;

Player::Player(glm::vec3 pos, Terrain &terrain)
    : Entity(pos), m_velocity(0,0,0), m_acceleration(0,0,0),
      m_camera(pos + glm::vec3(0, 1.5f, 0)), mcr_terrain(terrain),
      mcr_camera(m_camera),
      flight_mode(true),
      on_ground(false),
      rotateVelocity(vec2(0)),
      rotateAcceleration(vec2(1,1)),
      phi(0),
      theta(0)
{}

Player::~Player()
{}

void Player::tick(float dT, InputBundle &input) {
    processInputs(input);
    computePhysics(dT, mcr_terrain);
}

void Player::processInputs(InputBundle &inputs) {
    const float JUMP_AMOUNT = 0.055f;
    const float MAX_ANGLE_FROM_HORIZON = 70.f;
    // MOUSE CONTROLS

    // Mouse movement

    rotateOnUpGlobal(inputs.mouseX / 2);

    float prev_phi = phi;
    phi += inputs.mouseY / 2;
    if (phi > MAX_ANGLE_FROM_HORIZON) {
        phi = MAX_ANGLE_FROM_HORIZON;
    } else if(phi < -MAX_ANGLE_FROM_HORIZON) {
        phi = -MAX_ANGLE_FROM_HORIZON;
    } else {
        rotateOnRightLocal(phi - prev_phi);
    }

    // Block placing
    if (inputs.leftclickPressed) {
        removeBlock();
        inputs.leftclickPressed = false;
    }

    if (inputs.rightclickPressed) {
        addBlock(BlockType::STONE);
        inputs.rightclickPressed = false;
    }

    m_acceleration = vec3(0);

    // KEYBOARD CONTROLS

    if (inputs.wPressed) m_acceleration += flight_mode ? m_forward : glm::normalize(vec3(m_forward.x, 0, m_forward.z));
    if (inputs.sPressed) m_acceleration += flight_mode ? -m_forward : glm::normalize(vec3(-m_forward.x, 0, -m_forward.z));
    if (inputs.dPressed) m_acceleration += flight_mode ? m_right : glm::normalize(vec3(m_right.x, 0, m_right.z));
    if (inputs.aPressed) m_acceleration += flight_mode ? -m_right : glm::normalize(vec3(-m_right.x, 0, -m_right.z));
    if (inputs.ePressed) m_acceleration += flight_mode ? m_up : vec3(0);
    if (inputs.qPressed) m_acceleration += flight_mode ? -m_up : vec3(0);
    if (inputs.spacePressed && on_ground) m_velocity.y = flight_mode ? 0 : JUMP_AMOUNT;

    float acceleration_constant = 0.0001;
    //Set max acceleration (so we can't move faster by moving diagonally)
    if (glm::length(m_acceleration) > acceleration_constant) {
        m_acceleration = glm::normalize(m_acceleration) * acceleration_constant;
    }


}

void Player::computePhysics(float dT, const Terrain &terrain) {

    vec3 friction = -m_velocity * 0.01f;
    const vec3 GRAVITY = vec3(0, -0.0002, 0);

    //Friction + drag
    m_velocity += friction * dT;

    // Gravity
    if (!flight_mode) {
        m_velocity += GRAVITY * dT;
    }

    // Player movement acceleration
    m_velocity += (m_acceleration) * dT;

    vec3 move_ray = m_velocity * dT;

    if (!flight_mode) {
        detectCollisions(&move_ray, terrain);
    }

    // Move the player
    moveAlongVector(move_ray);
}

// Casts rays from each corner of the player's 1x2 cube
// We cast rays for each cardinal axis
void Player::detectCollisions(vec3 *ray_dir, const Terrain &terrain) {
    float out_dist = 0;
    ivec3 out_blockHit = ivec3();
    float axis;
    vec3 start = m_position - vec3(0.5,0,0.5);

    float min_outdist_x = INT_MAX;
    float min_outdist_y = INT_MAX;
    float min_outdist_z = INT_MAX;

    vec3 ray_dir_x = vec3(ray_dir->x, 0, 0);
    vec3 ray_dir_y = vec3(0, ray_dir->y, 0);
    vec3 ray_dir_z = vec3(0, 0,ray_dir->z);

    //Make collision box slightly smaller than 1x2 cube so we fit inside 1x1 holes
    float offset = 0.1;
    for (float x = 0 + offset; x <= 1; x += (1 - 2*offset)) {
        for (float z = 0 + offset; z <= 1; z += (1 - 2*offset)) {
            for (float y = 0 + offset; y <= 2.f - offset; y += (1 - offset)) {
                vec3 ray_origin = start + vec3(x,y,z);

                // Get the minimum distance for each cardinal axis
                if (gridMarch(ray_origin, ray_dir_x, terrain, &out_dist, &out_blockHit, &axis)) {
                    if (out_dist < min_outdist_x) {
                        min_outdist_x = out_dist;
                    }
                }

                if (gridMarch(ray_origin, ray_dir_y, terrain, &out_dist, &out_blockHit, &axis)) {
                    if (out_dist < min_outdist_y) {
                        min_outdist_y = out_dist;
                    }
                }

                if (gridMarch(ray_origin, ray_dir_z, terrain, &out_dist, &out_blockHit, &axis)) {
                    if (out_dist < min_outdist_z) {
                        min_outdist_z = out_dist;
                    }
                }
            }
        }
    }

    if (min_outdist_x != INT_MAX) {
        ray_dir_x *= min_outdist_x;
    }

    if (min_outdist_y < INT_MAX) {
        ray_dir_y *= min_outdist_y;

        // Check if we are currently standing on something, in which case we can jump
        if (abs(ray_dir_y.y) < 0.00005f) {
            if (sign(ray_dir_y.y) == -1) {
                on_ground = true;
            } else {
                on_ground = false;
            }
            ray_dir_y.y = 0;
        }
    } else {
        on_ground = false;
    }

    if (min_outdist_z != INT_MAX) {
        ray_dir_z *= min_outdist_z;
    }

    *ray_dir = (ray_dir_x + ray_dir_y + ray_dir_z);
}

// Removes a block wherever the player is looking
void Player::removeBlock() {
    vec3 ray_origin = m_camera.mcr_position;
    vec3 ray_dir = normalize(m_forward) * 3.f; // Multiply by 3f to set the range
    float out_dist = 0;
    ivec3 out_blockHit = ivec3();
    float axis;

    if (gridMarch(ray_origin, ray_dir, mcr_terrain, &out_dist, &out_blockHit, &axis)) {
        mcr_terrain.setBlockAt(out_blockHit.x, out_blockHit.y, out_blockHit.z, BlockType::EMPTY);
        mcr_terrain.getChunkAt(out_blockHit.x, out_blockHit.z)->createVBOdata();
    }
}

// Adds a stone block to where the player is looking
void Player::addBlock(const BlockType BLOCK_TYPE) {
    vec3 ray_origin = m_camera.mcr_position;
    vec3 ray_dir = normalize(m_forward) * 3.f; // Multiplyby 3f to set the range
    float out_dist = 0;
    ivec3 out_blockHit = ivec3();
    float axis;

    if (gridMarch(ray_origin, ray_dir, mcr_terrain, &out_dist, &out_blockHit, &axis)) {
        vec3 new_blockpos = vec3(out_blockHit.x - (axis == 0 ? sign(ray_dir.x) : 0),
                                 out_blockHit.y - (axis == 1 ? sign(ray_dir.y) : 0),
                                 out_blockHit.z - (axis == 2 ? sign(ray_dir.z) : 0));
        mcr_terrain.setBlockAt(new_blockpos.x, new_blockpos.y, new_blockpos.z, BLOCK_TYPE);
        mcr_terrain.getChunkAt(new_blockpos.x, new_blockpos.z)->createVBOdata();
    }
}

// Given in lecture
bool Player::gridMarch(vec3 rayOrigin, vec3 rayDirection, const Terrain &terrain, float *out_dist, ivec3 *out_blockHit, float *axis) {
    float maxLen = length(rayDirection); // Farthest we search
    ivec3 currCell = ivec3(floor(rayOrigin));
    rayDirection = normalize(rayDirection); // Now all t values represent world dist.

    float curr_t = 0.f;
    while(curr_t < maxLen) {
        float min_t = glm::sqrt(3.f);
        float interfaceAxis = -1; // Track axis for which t is smallest
        for(int i = 0; i < 3; ++i) { // Iterate over the three axes
            if(rayDirection[i] != 0) { // Is ray parallel to axis i?
                float offset = glm::max(0.f, glm::sign(rayDirection[i])); // See slide 5
                // If the player is *exactly* on an interface then
                // they'll never move if they're looking in a negative direction
                if(currCell[i] == rayOrigin[i] && offset == 0.f) {
                    offset = -1.f;
                }
                int nextIntercept = currCell[i] + offset;
                float axis_t = (nextIntercept - rayOrigin[i]) / rayDirection[i];
                axis_t = glm::min(axis_t, maxLen); // Clamp to max len to avoid super out of bounds errors
                if(axis_t < min_t) {
                    min_t = axis_t;
                    interfaceAxis = i;
                }
            }
        }
        if(interfaceAxis == -1) {
            throw std::out_of_range("interfaceAxis was -1 after the for loop in gridMarch!");
        }
        curr_t += min_t; // min_t is declared in slide 7 algorithm
        rayOrigin += rayDirection * min_t;
        ivec3 offset = ivec3(0,0,0);
        // Sets it to 0 if sign is +, -1 if sign is -
        offset[interfaceAxis] = glm::min(0.f, glm::sign(rayDirection[interfaceAxis]));
        currCell = ivec3(glm::floor(rayOrigin)) + offset;
        // If currCell contains something other than EMPTY, return
        // curr_t
        BlockType cellType = terrain.getBlockAt(currCell.x, currCell.y, currCell.z);
        if(cellType != BlockType::EMPTY) {
            *out_blockHit = currCell;
            *out_dist = glm::min(maxLen, curr_t);
            *axis = interfaceAxis;
            return true;
        }
    }
    *out_dist = glm::min(maxLen, curr_t);
    return false;
}

// Turns fligthmode on/off
void Player::toggleFlightMode() {
    m_velocity = vec3(0);
    flight_mode = !flight_mode;
}

void Player::setCameraWidthHeight(unsigned int w, unsigned int h) {
    m_camera.setWidthHeight(w, h);
}

void Player::moveAlongVector(glm::vec3 dir) {
    Entity::moveAlongVector(dir);
    m_camera.moveAlongVector(dir);
}
void Player::moveForwardLocal(float amount) {
    Entity::moveForwardLocal(amount);
    m_camera.moveForwardLocal(amount);
}
void Player::moveRightLocal(float amount) {
    Entity::moveRightLocal(amount);
    m_camera.moveRightLocal(amount);
}
void Player::moveUpLocal(float amount) {
    Entity::moveUpLocal(amount);
    m_camera.moveUpLocal(amount);
}
void Player::moveForwardGlobal(float amount) {
    Entity::moveForwardGlobal(amount);
    m_camera.moveForwardGlobal(amount);
}
void Player::moveRightGlobal(float amount) {
    Entity::moveRightGlobal(amount);
    m_camera.moveRightGlobal(amount);
}
void Player::moveUpGlobal(float amount) {
    Entity::moveUpGlobal(amount);
    m_camera.moveUpGlobal(amount);
}
void Player::rotateOnForwardLocal(float degrees) {
    Entity::rotateOnForwardLocal(degrees);
    m_camera.rotateOnForwardLocal(degrees);
}
void Player::rotateOnRightLocal(float degrees) {
    Entity::rotateOnRightLocal(degrees);
    m_camera.rotateOnRightLocal(degrees);
}
void Player::rotateOnUpLocal(float degrees) {
    Entity::rotateOnUpLocal(degrees);
    m_camera.rotateOnUpLocal(degrees);
}
void Player::rotateOnForwardGlobal(float degrees) {
    Entity::rotateOnForwardGlobal(degrees);
    m_camera.rotateOnForwardGlobal(degrees);
}
void Player::rotateOnRightGlobal(float degrees) {
    Entity::rotateOnRightGlobal(degrees);
    m_camera.rotateOnRightGlobal(degrees);
}
void Player::rotateOnUpGlobal(float degrees) {
    Entity::rotateOnUpGlobal(degrees);
    m_camera.rotateOnUpGlobal(degrees);
}

QString Player::posAsQString() const {
    std::string str("( " + std::to_string(m_position.x) + ", " + std::to_string(m_position.y) + ", " + std::to_string(m_position.z) + ")");
    return QString::fromStdString(str);
}
QString Player::velAsQString() const {
    std::string str("( " + std::to_string(m_velocity.x) + ", " + std::to_string(m_velocity.y) + ", " + std::to_string(m_velocity.z) + ")");
    return QString::fromStdString(str);
}
QString Player::accAsQString() const {
    std::string str("( " + std::to_string(m_acceleration.x) + ", " + std::to_string(m_acceleration.y) + ", " + std::to_string(m_acceleration.z) + ")");
    return QString::fromStdString(str);
}
QString Player::lookAsQString() const {
    std::string str("( " + std::to_string(m_forward.x) + ", " + std::to_string(m_forward.y) + ", " + std::to_string(m_forward.z) + ")");
    return QString::fromStdString(str);
}
