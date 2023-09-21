#define COW_PATCH_FRAMERATE
#include "include.cpp"

#define NUM_OF_BULLETS 1000
struct Bullet {
    vec2 position;
    bool live;
    vec2 at_time_of_shot;
    int age_in_frames;

    void update(real speed) {
        position += speed * at_time_of_shot;
        age_in_frames += 1;
    }

    int check_age() {
        if (age_in_frames > 500) {
            live = false;
            age_in_frames = 0;
            return 1;
        }
        return 0;
    }

    void set() {
        position = V2(0.0, 0.0);
        live = false;
        at_time_of_shot = V2(0.0, 0.0);
        age_in_frames = 0;
    }
};

void cleanup(Bullet *bullets, int save_last_bullet) {
    int j = 0;
    for (int i = 0; i < save_last_bullet; i++) {
        if(!bullets[i].live) {
            j++;
            if(bullets[j].live) {
                Bullet temp;
                temp = bullets[j];
                bullets[j] = bullets[i];
                bullets[i] = temp;
            } else {
                while(!bullets[j].live) {
                    j++;
                }
            }
        }
    }
}

void cleanup(Bullet *bullets, int size, int version) {
    for (int i = 0; i < size; i++) {
        if(!bullets[i].live) {
            bullets[i].set();
        }
    }
}

void reset(Bullet *bullets, int num) {
    for(int i = 0; i < num; i++) {
        bullets[i].set();
    }
}

struct Enemy {
    int type;
    int num_bullets;
    Bullet bullets[10];

    bool alive;
    real health;
    vec2 move_direction;
    real speed;
    int row_num;
    IndexedTriangleMesh3D skin;

    vec2 position_center;
    real radius;
    real moved_down;
    bool hit_right_wall;
    bool hit_left_wall;
    bool hit_wall;

    vec2 hitbox_positions[4];

    bool collideWithPoint(vec2 bullet) {
        return alive && bullet.x > hitbox_positions[3].x && bullet.x < hitbox_positions[2].x 
                     && bullet.y > hitbox_positions[3].y && bullet.y < hitbox_positions[0].y;
    }

    bool hits_right_wall() {
        return (position_center.x + 0.13 > 1.4);
    }

    bool hits_left_wall() {
        return (position_center.x - 0.13 < -1.45);
    }

    void update() {
        position_center += speed * move_direction;
        update_hitbox();
        update_skin_vertices();

        // update skin here
    }

    void update_hitbox() {
        hitbox_positions[0] = V2(position_center.x - 0.13, position_center.y + 0.13);
        hitbox_positions[1] = V2(position_center.x + 0.13, position_center.y + 0.13);
        hitbox_positions[2] = V2(position_center.x + 0.13, position_center.y - 0.13);
        hitbox_positions[3] = V2(position_center.x - 0.13, position_center.y - 0.13);
    }

    void update_skin_vertices() {
        skin.vertex_positions[0] = V3(position_center.x - 0.13, position_center.y + 0.13, 0.0);
        skin.vertex_positions[1] = V3(position_center.x + 0.13, position_center.y + 0.13, 0.0);
        skin.vertex_positions[2] = V3(position_center.x + 0.13, position_center.y - 0.13, 0.0);
        skin.vertex_positions[3] = V3(position_center.x - 0.13, position_center.y - 0.13, 0.0);
    }

    void reset() {
        alive = false;
    }

    bool reaches_player(real player_y) {
        return player_y > position_center.y;
    }
};

real determine_health(int i) {
    if(i == 2) {
        return 1.0;
    } else if (i == 1) {
        return 0.5;
    } 
    return 0.25;
}

void set_enemy_texture(Enemy *enemies, int num) {
    IndexedTriangleMesh3D texture = {};
    switch (enemies[num].type) {
        case 0:
            texture.num_vertices = 4;
            texture.num_triangles = 2;
            texture.vertex_positions = (vec3*) malloc(6 * sizeof(vec3));
            texture.triangle_indices = (int3*) malloc(2 * sizeof(int3));
            texture.triangle_indices[0] = {3, 2, 1};
            texture.triangle_indices[1] = {0, 1, 3};
            texture.vertex_texture_coordinates = library.meshes.square.vertex_texture_coordinates;
            texture.texture_filename = "bluealien.png";
            enemies[num].skin = texture;
            break;

        case 1:
            texture.num_vertices = 4;
            texture.num_triangles = 2;
            texture.vertex_positions = (vec3*) malloc(6 * sizeof(vec3));
            texture.triangle_indices = (int3*) malloc(2 * sizeof(int3));
            texture.triangle_indices[0] = {3, 2, 1};
            texture.triangle_indices[1] = {0, 1, 3};
            texture.vertex_texture_coordinates = library.meshes.square.vertex_texture_coordinates;
            texture.texture_filename = "octoalien.png";
            enemies[num].skin = texture;
            break;
        
        case 2:
            texture.num_vertices = 4;
            texture.num_triangles = 2;
            texture.vertex_positions = (vec3*) malloc(6 * sizeof(vec3));
            texture.triangle_indices = (int3*) malloc(2 * sizeof(int3));
            texture.triangle_indices[0] = {3, 2, 1};
            texture.triangle_indices[1] = {0, 1, 3};
            texture.vertex_texture_coordinates = library.meshes.square.vertex_texture_coordinates;
            texture.texture_filename = "squidalien.png";
            enemies[num].skin = texture;
            break;
        // default:
        //     // enemies[i].skin = NULL;
        //     break;
    }
}

void set_enemies(Enemy *enemies, int num, int lvl[]) {

    int num_rows = num / 11;
    if (num_rows < 1) {
        num_rows = 1;
    }
    int row_num = 0;
    vec2 move = V2(0.1, 0.0);

    for(int i = 0; i < num; i++) {

        enemies[i].position_center = V2(0.0, 0.0);
        enemies[i].position_center.x = LINEAR_REMAP(i % 11, 0, 11, -1.3, 1.3);
        if (i % 11 == 0) {
            row_num += 1;
            move = V2(-move.x, 0.0);
        }
        // enemies[i].position_center.y = LINEAR_REMAP(row_num, 0, num_rows, 0.5, 0.9);
        // enemies[i].position_center.y = LERP(row_num, 0, num_rows);
        enemies[i].position_center.y = LINEAR_REMAP(row_num, 0, num_rows, 0.3, 0.3 + (num_rows * 0.2));

        enemies[i].update_hitbox();
        enemies[i].row_num = row_num;
        enemies[i].moved_down = 0.0;
        enemies[i].hit_left_wall = false;
        enemies[i].hit_right_wall = false;
        enemies[i].hit_wall = false;
        enemies[i].speed = 0.025;

        enemies[i].type = lvl[i];

        enemies[i].radius = 0.1;
        enemies[i].health = determine_health(enemies[i].type);
        enemies[i].move_direction = move;
        enemies[i].alive = true;
        enemies[i].num_bullets = 10;
        for(int j = 0; j < enemies[i].num_bullets; j++) {
            enemies[i].bullets[j].set();
        }

        set_enemy_texture(enemies, i);

        enemies[i].update_skin_vertices();
    }
}

void move_enemies(Enemy *enemies, int num_of_enemies, int version) {
    for (int i = 0; i < num_of_enemies; i++) {

        real to_move_down = 0.0;
        if (version == 0) {
            to_move_down = 8.0;
        } else if (version = 1) {
            to_move_down = 4.0;
        }

        if (enemies[i].hits_left_wall()) {
            enemies[i].hit_left_wall = true;
            enemies[i].hit_wall = true;
            // break;
        }

        if (enemies[i].hits_right_wall()) {
            enemies[i].hit_right_wall = true;
            enemies[i].hit_wall = true;
            // break;
        }

        if(enemies[i].hit_wall) {
            enemies[i].move_direction = V2(0.0, -0.05);
            // break;
        }

        if (enemies[i].move_direction.y < 0.0) {
            enemies[i].moved_down += 0.05;
        }

        if (enemies[i].moved_down >= to_move_down && enemies[i].hit_left_wall) {
            enemies[i].move_direction = V2(0.1, 0.0);
            enemies[i].moved_down = 0.0;
            enemies[i].hit_wall = false;
            enemies[i].hit_left_wall = false;
        }

        if (enemies[i].moved_down >= to_move_down && enemies[i].hit_right_wall) {
            enemies[i].move_direction = V2(-0.1, 0.0);
            enemies[i].moved_down = 0.0;
            enemies[i].hit_wall = false;
            enemies[i].hit_right_wall = false;
        }

        if (enemies[i].health > 0 && enemies[i].alive) {
            enemies[i].update(); 
        } else {
            enemies[i].reset();
        }
    }
}

void switch_enemy_textures(Enemy *enemies, int num, bool sign) {
    if (sign) {
        for(int i = 0; i < num; i++) {
            switch(enemies[i].type) {
                case 0:
                    enemies[i].skin.texture_filename = "bluealien.png";
                    break;

                case 1:
                    enemies[i].skin.texture_filename = "octoalien.png";
                    break;

                case 2:
                    enemies[i].skin.texture_filename = "squidalien.png";
                    break;
            }
        }
    } else if (!sign) {
        for(int i = 0; i < num; i++) {
            switch(enemies[i].type) {
                case 0:
                    enemies[i].skin.texture_filename = "bluealienmoving.png";
                    break;
                
                case 1:
                    enemies[i].skin.texture_filename = "octoalienmoving.png";
                    break;
                
                case 2:
                    enemies[i].skin.texture_filename = "squidalienmoving.png";
                    break;
            }
        }
    }
        
}

bool flip(int counter, bool animate) {
    if (counter == 0 && animate) {
        return false;
    } else if (counter == 0 && !animate) {
        return true;
    }
    return false;
}

struct Player {
    vec2 position[6]; // where [3] is the center
    real health;
    IndexedTriangleMesh3D ship_tex = {};

    void move(char ad) {
        if (ad == 'a' && position[0].x > -1.5) {
            for (int i = 0; i < 6; i++) {
                position[i].x = position[i].x - 0.01;
            }
            for (int i = 0; i < 4; i++) {
                ship_tex.vertex_positions[i].x = ship_tex.vertex_positions[i].x - 0.01;
            }
        } else if (ad == 'd' && position[1].x < 1.5) {
            for (int i = 0; i < 6; i++) {
                position[i].x = position[i].x + 0.01;
            }
            for (int i = 0; i < 4; i++) {
                ship_tex.vertex_positions[i].x = ship_tex.vertex_positions[i].x + 0.01;
            }
        } 
    }

    void reset() {
        position[0] = V2(-0.1, -1.3);
        position[1] = V2(0.1, -1.3);
        position[2] = V2(-0.1, -1.1);
        position[3] = V2(0.1, -1.1);
        position[4] = V2(0.0, -1.2); // center
        position[5] = V2(0.0, -1.1); // facing direction
        health = 1.0;

        ship_tex.num_vertices = 4;
        ship_tex.num_triangles = 2;
        ship_tex.vertex_positions = (vec3*) malloc(6 * sizeof(vec3));
        ship_tex.vertex_positions[0] = V3(-0.1, -1.3, 0.0);
        ship_tex.vertex_positions[3] = V3(0.1, -1.3, 0.0);
        ship_tex.vertex_positions[1] = V3(-0.1, -1.1, 0.0);
        ship_tex.vertex_positions[2] = V3(0.1, -1.1, 0.0);
        ship_tex.vertex_positions[4] = V3(0.0, -1.2, 0.0); // center
        ship_tex.vertex_positions[5] = V3(0.0, -1.1, 0.0); // facing direction
        ship_tex.triangle_indices = (int3*) malloc(2 * sizeof(int3));
        ship_tex.triangle_indices[0] = {3, 2, 1};
        ship_tex.triangle_indices[1] = {0, 1, 3};
        ship_tex.vertex_texture_coordinates = library.meshes.square.vertex_texture_coordinates;
        ship_tex.texture_filename = "spaceship.png";

    }

    bool collidesWithBullet(Bullet bullet) {
        real origArea = 0.0;
        real area1 = 0.0;
        real area2 = 0.0;
        real area3 = 0.0;

        vec2 bulletpos = bullet.position;
        vec2 one = position[0];
        vec2 two = position[3];
        vec2 three = position[5];
        
        origArea = abs((two.x - one.x) * (three.y - one.y) - (three.x - one.x) * (two.y - one.y));

        area1 = abs((one.x - bulletpos.x)   * (two.y - bulletpos.y)   - (two.x - bulletpos.x)   * (one.y - bulletpos.y));
        area2 = abs((two.x - bulletpos.x)   * (three.y - bulletpos.y) - (three.x - bulletpos.x) * (two.y - bulletpos.y));
        area3 = abs((three.x - bulletpos.x) * (one.y - bulletpos.y)   - (one.x - bulletpos.x)   * (three.y - bulletpos.y));

        return origArea == area1 + area2 + area3;
    }
};

void invaders() {
    Camera2D camera = { 3.0 };

    Player *player = (Player *) malloc(1 * sizeof(Player));
    player[0].reset();

                         // level 1
    int levels[4][36] =  { { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
                             0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },

                         // level 2
                           { 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 
                             0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },

                         // level 3
                           { 2, 1, 0, 1, 0, 1, 0, 1, 0, 1, 2,
                             0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                             0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },

                           { 2, 2, 1, 1, 1, 1, 1, 1, 1, 2, 2,
                             1, 1, 0, 0, 0, 0, 0, 0, 0, 1, 1 }
                        };
    
    int level_size[4] = {22, 22, 33, 22};


    Bullet *bullets = (Bullet *) malloc(NUM_OF_BULLETS * sizeof(Bullet));
    for (int i = 0; i < NUM_OF_BULLETS; i++) {
        bullets[i].position = V2(0.0, 0.0);
        bullets[i].live = false;
        bullets[i].age_in_frames = 0;
        bullets[i].at_time_of_shot = V2(0.0, 0.0);
    }

    real time = 0.0;
    real max_speed = 0.1;
    int cooldown_to_shoot = 10;
    int live_bullets = 0;
    int live_bullets_enemies = 0;
    bool enemy_death = false;

    int all_dead = 0;
    int level = 0;

    int num_of_enemies = level_size[level];
    Enemy *enemies = (Enemy *) malloc(num_of_enemies * sizeof(Enemy));
    set_enemies(enemies, level_size[level], levels[level]);

    int enemy_mover_cooldown = 5;
    int enemy_shot_cooldown = 60; 
    int enemy_animator_cooldown = 60;   
    bool animate = true;   
    
    vec2 *walls = (vec2 *) malloc(4 * sizeof(vec2));
    walls[0] = V2(1.5, -1.0);
    walls[1] = V2(1.5, 1.0);
    walls[2] = V2(-1.5, -1.0);
    walls[3] = V2(-1.5, 1.0);

    bool restart = false;

    IndexedTriangleMesh3D background = {};
    background.num_vertices = 4;
    background.num_triangles = 2;
    background.vertex_positions = (vec3*) malloc(4 * sizeof(vec3));
    background.vertex_positions[0] = V3(-4.0, 4.0, 0.0);
    background.vertex_positions[1] = V3(4.0, 4.0, 0.0);
    background.vertex_positions[2] = V3(4.0, -4.0, 0.0);
    background.vertex_positions[3] = V3(-4.0, -4.0, 0.0);
    background.triangle_indices = (int3*) malloc(2 * sizeof(int3));
    background.triangle_indices[0] = {3, 2, 1};
    background.triangle_indices[1] = {0, 1, 3};
    background.vertex_texture_coordinates = (vec2*) malloc (4 * sizeof(vec2));
    background.vertex_texture_coordinates[0] = V2(-4.0, 4.0);
    background.vertex_texture_coordinates[1] = V2(4.0, 4.0);
    background.vertex_texture_coordinates[2] = V2(4.0, -4.0);
    background.vertex_texture_coordinates[3] = V2(-4.0, -4.0);
    background.texture_filename = "background.png";    

    while (cow_begin_frame()) {
        camera_move(&camera);
        mat4 PV = camera_get_PV(&camera);
        num_of_enemies = level_size[level];

        // mesh_draw(PV, globals.Identity, globals.Identity, background.num_triangles, background.triangle_indices,
        //               background.num_vertices, background.vertex_positions, NULL, NULL, monokai.white,
        //               background.vertex_texture_coordinates, "background.png");
        
        if (level < 4 && !all_dead) {
            
            gui_readout("player_health:", &player[0].health);
            gui_readout("num enemies:", &num_of_enemies);
            gui_readout("level:", &level);
            gui_readout("alien moved", &enemies[0].moved_down);

            if (gui_button("reset")) {
                time = 0.0;
                player[0].reset();

                for (int i = 0; i < NUM_OF_BULLETS; i++) {
                    bullets[i].live = false;
                }

                live_bullets = 0;
                
                level = 0;
                set_enemies(enemies, level_size[level], levels[level]);
            }

            time += .0167;
            { // cooldowns
                if (cooldown_to_shoot > -10) {
                    cooldown_to_shoot -= 1;
                }
                if (enemy_mover_cooldown > 0) {
                    enemy_mover_cooldown -= 1;
                }
                if (enemy_shot_cooldown > 0) {
                    enemy_shot_cooldown -= 1;
                }
                if(enemy_animator_cooldown > 0) {
                    enemy_animator_cooldown -= 1;
                }
            }
            
            { // player controls
                if (globals.key_pressed['a'] || globals.key_held['a']) {
                player[0].move('a'); 
                } else if (globals.key_pressed['d'] || globals.key_held['d']) {
                    player[0].move('d'); 
                } 
            }

            { // checking if bullets affect things
                int save_last_bullet = 0;
                // player shooting bullets      
                if ((globals.key_pressed[COW_KEY_SPACE] || globals.key_held[COW_KEY_SPACE]) && cooldown_to_shoot < 0) {
                    
                    // find where in the array there is a not live bullet and populate it there
                    for (int i = 0; i < NUM_OF_BULLETS; i++) {
                        if (!bullets[i].live) {
                            bullets[i].position = V2(player[0].position[5].x, player[0].position[5].y + 0.02);
                            bullets[i].live = true;
                            // creates direction vector using information of what direction
                            // the ship is facing
                            bullets[i].at_time_of_shot = player[0].position[5] - player[0].position[4];
                            live_bullets += 1;
                            save_last_bullet = i;
                            break;
                        }
                    }
                    cooldown_to_shoot = 10;
                }

                cleanup(bullets, save_last_bullet);
                
                // checking if player bullets hit enemies
                for (int i = 0; i < NUM_OF_BULLETS; i++) {
                    if (bullets[i].live) {
                        for (int j = 0; j < num_of_enemies; j++) {
                            if (enemies[j].collideWithPoint(bullets[i].position)) {
                                enemies[j].health -= 0.25;
                                if (enemies[j].health <= 0.0) {
                                    enemy_death = true;
                                }
                                bullets[i].live = false;
                                live_bullets--;
                                break;
                            }
                        }
                    }
                }

                // checking if enemy bullets hit the player
                for (int i = 0; i < num_of_enemies; i++) {
                    for(int j = 0; j < 5; j++) {
                        if (enemies[i].bullets[j].live && player[0].collidesWithBullet(enemies[i].bullets[j])) {
                            player[0].health -= 0.1;
                            enemies[i].bullets[j].live = false;
                        }
                    }
                }
            }

            { // bullets update and draw
                for (int i = 0; i < NUM_OF_BULLETS; i++) {
                    if (bullets[i].live) {
                        bullets[i].update(max_speed);
                        eso_begin(PV, SOUP_LINES, 8.0, false);
                        eso_color(monokai.red);
                        eso_vertex(V2(bullets[i].position.x, bullets[i].position.y + 0.03));
                        eso_vertex(V2(bullets[i].position.x, bullets[i].position.y - 0.03));
                        eso_end();
                    }
                    live_bullets -= bullets[i].check_age();
                }

                int save_last_bullet_enemy = 0;
                for (int i = 0; i < num_of_enemies; i++) {
                    for (int j = 0; j < enemies[i].num_bullets; j++) {
                        if (enemies[i].bullets[j].live) {
                            enemies[i].bullets[j].update(max_speed);
                            eso_begin(PV, SOUP_LINES, 8.0, false);
                            eso_color(monokai.white);
                            if (enemies[i].bullets[j].at_time_of_shot.x > 0.01) {
                                eso_vertex(V2(enemies[i].bullets[j].position.x - 0.03, enemies[i].bullets[j].position.y + 0.03));
                                eso_vertex(V2(enemies[i].bullets[j].position.x + 0.03, enemies[i].bullets[j].position.y - 0.03));
                            } else if(enemies[i].bullets[j].at_time_of_shot.x < 0.01 && enemies[i].bullets[j].at_time_of_shot.x > -0.01) {
                                eso_vertex(V2(enemies[i].bullets[j].position.x, enemies[i].bullets[j].position.y + 0.03));
                                eso_vertex(V2(enemies[i].bullets[j].position.x, enemies[i].bullets[j].position.y - 0.03));
                            } else {
                                eso_vertex(V2(enemies[i].bullets[j].position.x + 0.03, enemies[i].bullets[j].position.y + 0.03));
                                eso_vertex(V2(enemies[i].bullets[j].position.x - 0.03, enemies[i].bullets[j].position.y - 0.03));
                            }
                            eso_end();
                            save_last_bullet_enemy = j;
                        }
                        live_bullets_enemies -= enemies[i].bullets[j].check_age();
                        cleanup(enemies[i].bullets, enemies[i].num_bullets, 1);
                    }
                }
            }

            { // draw player and enemies
                mesh_draw(PV, globals.Identity, globals.Identity, player[0].ship_tex.num_triangles, 
                    player[0].ship_tex.triangle_indices, player[0].ship_tex.num_vertices, 
                    player[0].ship_tex.vertex_positions, NULL, NULL, monokai.red, 
                    player[0].ship_tex.vertex_texture_coordinates, "spaceship.png");

                for(int i = 0; i < num_of_enemies; i++) {
                    if (enemies[i].alive) {
                        mesh_draw(PV, globals.Identity, globals.Identity, enemies[i].skin.num_triangles, 
                            enemies[i].skin.triangle_indices, enemies[i].skin.num_vertices, 
                            enemies[i].skin.vertex_positions, NULL, NULL, monokai.red, 
                            enemies[i].skin.vertex_texture_coordinates, enemies[i].skin.texture_filename);
                    }
                }
            }

            { // enemy cooldowns - movement, shooting, & animation
                if (enemy_mover_cooldown <= 0) {
                    if (level == 0) {
                        move_enemies(enemies, num_of_enemies, 0);
                    } else {
                        move_enemies(enemies, num_of_enemies, 1);
                    }
                    enemy_mover_cooldown = 5;
                }

                // random shooting
                int enemy_shooter = 0;
                if (enemy_shot_cooldown <= 0) {
                    enemy_shooter = (int) random_real(0.0, (real)num_of_enemies);
                    while (!enemies[enemy_shooter].alive) {
                        enemy_shooter = (int) random_real(0.0, (real)num_of_enemies);
                    }

                    if (enemies[enemy_shooter].type == 0) {
                        int check = 0;
                        for (int i = 0; i < enemies[enemy_shooter].num_bullets; i++) {
                            if(!enemies[enemy_shooter].bullets[i].live) {
                                check = i;
                                break;
                            }
                        }
                        enemies[enemy_shooter].bullets[(check) % 10].position = enemies[enemy_shooter].position_center;
                        enemies[enemy_shooter].bullets[(check) % 10].live = true;
                        enemies[enemy_shooter].bullets[(check) % 10].age_in_frames = 0;
                        enemies[enemy_shooter].bullets[(check) % 10].at_time_of_shot = V2(enemies[enemy_shooter].position_center.x, enemies[enemy_shooter].position_center.y - 0.1) - enemies[enemy_shooter].position_center;
                        enemy_shot_cooldown = 60;
                        live_bullets_enemies++;

                    } else if (enemies[enemy_shooter].type == 1) {
                        int check = 0;
                        for (int i = 0; i < enemies[enemy_shooter].num_bullets; i++) {
                            if(!enemies[enemy_shooter].bullets[i].live) {
                                check = i;
                                break;
                            }
                        }
                        enemies[enemy_shooter].bullets[(check) % 10].position = enemies[enemy_shooter].position_center;
                        enemies[enemy_shooter].bullets[(check) % 10].live = true;
                        enemies[enemy_shooter].bullets[(check) % 10].age_in_frames = 0;
                        enemies[enemy_shooter].bullets[(check) % 10].at_time_of_shot = enemies[enemy_shooter].hitbox_positions[2] - enemies[enemy_shooter].position_center;
                        live_bullets_enemies++;

                        enemies[enemy_shooter].bullets[(check + 1) % 10].position = enemies[enemy_shooter].position_center;
                        enemies[enemy_shooter].bullets[(check + 1) % 10].live = true;
                        enemies[enemy_shooter].bullets[(check + 1) % 10].age_in_frames = 0;
                        enemies[enemy_shooter].bullets[(check + 1) % 10].at_time_of_shot = enemies[enemy_shooter].hitbox_positions[3] - enemies[enemy_shooter].position_center;
                        live_bullets_enemies++;

                        enemy_shot_cooldown = 60;
                    } else if (enemies[enemy_shooter].type == 2) {
                        int check = 0;
                        for (int i = 0; i < enemies[enemy_shooter].num_bullets; i++) {
                            if(!enemies[enemy_shooter].bullets[i].live) {
                                check = i;
                                break;
                            }
                        }
                        enemies[enemy_shooter].bullets[(check + 1) % 10].position = enemies[enemy_shooter].position_center;
                        enemies[enemy_shooter].bullets[(check + 1) % 10].live = true;
                        enemies[enemy_shooter].bullets[(check + 1) % 10].age_in_frames = 0;
                        enemies[enemy_shooter].bullets[(check + 1) % 10].at_time_of_shot = enemies[enemy_shooter].hitbox_positions[2] - enemies[enemy_shooter].position_center;
                        live_bullets_enemies++;

                        enemies[enemy_shooter].bullets[(check + 2) % 10].position = enemies[enemy_shooter].position_center;
                        enemies[enemy_shooter].bullets[(check + 2) % 10].live = true;
                        enemies[enemy_shooter].bullets[(check + 2) % 10].age_in_frames = 0;
                        enemies[enemy_shooter].bullets[(check + 2) % 10].at_time_of_shot = enemies[enemy_shooter].hitbox_positions[3] - enemies[enemy_shooter].position_center;
                        live_bullets_enemies++;

                        enemies[enemy_shooter].bullets[(check) % 10].position = enemies[enemy_shooter].position_center;
                        enemies[enemy_shooter].bullets[(check) % 10].live = true;
                        enemies[enemy_shooter].bullets[(check) % 10].age_in_frames = 0;
                        enemies[enemy_shooter].bullets[(check) % 10].at_time_of_shot = V2(enemies[enemy_shooter].position_center.x, enemies[enemy_shooter].position_center.y - 0.1) - enemies[enemy_shooter].position_center;
                        live_bullets_enemies++;

                        enemy_shot_cooldown = 60;
                        
                    }

                }

                // animation
                if (enemy_animator_cooldown <= 0) {
                    switch_enemy_textures(enemies, num_of_enemies, animate);

                    animate = flip(enemy_animator_cooldown, animate);
                    enemy_animator_cooldown = 60;
                }

                for (int i = 0; i < num_of_enemies; i++) {
                    if(!enemies[i].alive) {
                        all_dead = 1;
                    } else {
                        all_dead = 0;
                        break;
                    }
                }
            }

            { // draw barriers
                eso_begin(PV, SOUP_LINES, 4.0, false);
                eso_color(monokai.white);
                for(int i = 0; i < 4; i++) {
                    eso_vertex(walls[i]);
                }
                eso_end();
            }
        
            { // losing
                real y = player[0].position[3].y;
                for (int i = 0; i < num_of_enemies; i++) {
                    if (player[0].health <= 0.0 || enemies[i].reaches_player(y)) {
                        restart = true;
                    }
                }

                if (restart) {
                    free(enemies);
                    free(bullets);
                    free(player);

                    all_dead = 0;
                    level = 0;
                    time = 0.0;

                    enemies = (Enemy *) malloc(level_size[level] * sizeof(Enemy));
                    bullets = (Bullet *) malloc(NUM_OF_BULLETS * sizeof(Bullet));
                    player = (Player *) malloc(1 * sizeof(Player));

                    set_enemies(enemies, level_size[level], levels[level]);
                    for(int j = 0; j < level_size[level]; j++) {
                        enemies[j].speed += 0.025;
                    }

                    reset(bullets, NUM_OF_BULLETS);
                    live_bullets = 0;
                    
                    player[0].reset();
                    restart = false;
                }
            }
        }

        // next level
        if (all_dead) {
            free(enemies);
            free(bullets);
            free(player);

            all_dead = 0;
            time = 0.0;

            if(level == 3) {
                level = 3;
            } else {
                level += 1;
            }

            enemies = (Enemy *) malloc(level_size[level] * sizeof(Enemy));
            bullets = (Bullet *) malloc(NUM_OF_BULLETS * sizeof(Bullet));
            player = (Player *) malloc(1 * sizeof(Player));

            set_enemies(enemies, level_size[level], levels[level]);
            for(int i = 0; i < level_size[level]; i++) {
                enemies[i].speed += 0.025;
            }

            reset(bullets, NUM_OF_BULLETS);
            live_bullets = 0;

            player[0].reset();
        }
    }

    free(bullets);
    free(walls);
    free(enemies);
    free(player);
}

// MENU
// SCORE
// more levels
// GAME OVER SCREEN?
// UFO & Powerup/down textures and functionality
// background

int main() {
    APPS {
        APP(invaders);
    }
    return 0;
}