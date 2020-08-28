#include <stdio.h>
#include <stdlib.h>
#include "vsi.h"
#include "simvars.h"

vsi::vsi(int xPos, int yPos, int size) : instrument(xPos, yPos, size)
{
    setName("VSI");
    addVars();
    resize();
}

/// <summary>
/// Destroy and recreate all bitmaps as instrument has been resized
/// </summary>
void vsi::resize()
{
    destroyBitmaps();

    // Create bitmaps scaled to correct size (original size is 800)
    scaleFactor = size / 800.0f;

    // 0 = Original (loaded) bitmap
    ALLEGRO_BITMAP* orig = loadBitmap("vsi.bmp");
    addBitmap(orig);

    if (bitmaps[0] == NULL) {
        return;
    }

    // 1 = Destination bitmap (all other bitmaps get assembled to here)
    ALLEGRO_BITMAP* bmp = al_create_bitmap(size, size);
    addBitmap(bmp);

    // 2 = Main dial
    bmp = al_create_bitmap(size, size);
    al_set_target_bitmap(bmp);
    al_draw_scaled_bitmap(orig, 0, 0, 800, 800, 0, 0, size, size, 0);
    addBitmap(bmp);

    // 3 = Pointer
    bmp = al_create_bitmap(800, 100);
    al_set_target_bitmap(bmp);
    al_draw_bitmap_region(orig, 0, 800, 800, 100, 0, 0, 0);
    addBitmap(bmp);

    // 4 = Pointer shadow
    bmp = al_create_bitmap(800, 100);
    al_set_target_bitmap(bmp);
    al_draw_bitmap_region(orig, 0, 900, 800, 100, 0, 0, 0);
    addBitmap(bmp);

    al_set_target_backbuffer(globals.display);
}

/// <summary>
/// Draw the instrument at the stored position
/// </summary>
void vsi::render()
{
    if (bitmaps[0] == NULL) {
        return;
    }

    // Use normal blender
    al_set_blender(ALLEGRO_ADD, ALLEGRO_ALPHA, ALLEGRO_INVERSE_ALPHA);

    // Draw stuff into dest bitmap
    al_set_target_bitmap(bitmaps[1]);

    // Add main dial
    al_draw_bitmap(bitmaps[2], 0, 0, 0);

    if (globals.enableShadows) {
        // Set blender to multiply (shades of grey darken, white has no effect)
        al_set_blender(ALLEGRO_ADD, ALLEGRO_DEST_COLOR, ALLEGRO_ZERO);

        // Add pointer shadow
        al_draw_scaled_rotated_bitmap(bitmaps[4], 400,50, 415 * scaleFactor, 415 * scaleFactor, scaleFactor, scaleFactor, angle * AngleFactor, 0);

        // Restore normal blender
        al_set_blender(ALLEGRO_ADD, ALLEGRO_ALPHA, ALLEGRO_INVERSE_ALPHA);
    }

    // Add pointer
    al_draw_scaled_rotated_bitmap(bitmaps[3], 400, 50, 400 * scaleFactor, 400 * scaleFactor, scaleFactor, scaleFactor, angle * AngleFactor, 0);

    // Position dest bitmap on screen
    al_set_target_backbuffer(globals.display);
    al_draw_bitmap(bitmaps[1], xPos, yPos, 0);
}

/// <summary>
/// Fetch flightsim vars and then update all internal variables
/// that affect this instrument.
/// </summary>
void vsi::update()
{
    // Check for position or size change
    long *settings = globals.simVars->readSettings(name, xPos, yPos, size);

    xPos = settings[0];
    yPos = settings[1];

    if (size != settings[2]) {
        size = settings[2];
        resize();
    }

    // Get latest FlightSim variables
    SimVars* simVars = &globals.simVars->simVars;

    // Calculate values
    targetAngle = simVars->vsiVerticalSpeed / 10.f;

    float diff = abs(targetAngle - angle);

    if (diff > 10.0f) {
        if (angle < targetAngle) angle += 1.0f; else angle -= 1.0f;
    }
    else if (diff > 5.0f) {
        if (angle < targetAngle) angle += 0.5f; else angle -= 0.5f;
    }
    else if (diff > 2.5f) {
        if (angle < targetAngle) angle += 0.25f; else angle -= 0.25f;
    }
    else if (diff > 1.25f) {
        if (angle < targetAngle) angle += 0.125f; else angle -= 0.125f;
    }
    else if (diff > 0.625f) {
        if (angle < targetAngle) angle += 0.0625f; else angle -= 0.0625f;
    }
    else if (diff > 0.03f) {
        if (angle < targetAngle) angle += 0.03f; else angle -= 0.03f;
    }
    else {
        angle = targetAngle;
    }
}

/// <summary>
/// Add FlightSim variables for this instrument (used for simulation mode)
/// </summary>
void vsi::addVars()
{
    globals.simVars->addVar(name, "Vertical Speed", false, 4, 0);
}