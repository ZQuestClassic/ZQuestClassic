class Trigger: enemy
{
    Trigger()
    {
        hxofs=1000;
    }
    
    void Draw()
    {
        update_enemy_frame();
        enemy::draw();
    }
}

