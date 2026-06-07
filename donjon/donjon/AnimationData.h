#pragma once
#include <SFML/Graphics.hpp>
#include <vector>
#include <string>

struct AnimationData {
    std::vector<sf::IntRect> frames;
    const sf::Texture* textureTarget{ nullptr };
    float frameDuration{ 0.1f };
    bool bLoops{ true };

    static AnimationData detectFlipbook(const std::string& texturePath, float totalDuration, bool loops = true) {
        AnimationData anim;
        anim.bLoops = loops;

        anim.textureTarget = &SmartTextureLoader::getTexture(texturePath);

        sf::Image image;
        if (!image.loadFromFile(texturePath)) {
            return anim;
        }

        sf::Vector2u size = image.getSize();
        if (size.x == 0 || size.y == 0) return anim;

        unsigned int safeHeight = size.y;
        std::vector<int> density(size.x, 0);

        for (unsigned int x = 0; x < size.x; ++x) {
            for (unsigned int y = 0; y < safeHeight; ++y) {
                if (image.getPixel({ x, y }).a > 0) {
                    density[x]++;
                }
            }
        }

        int blurRadius = 12;
        std::vector<int> smoothed(size.x, 0);
        for (int x = 0; x < static_cast<int>(size.x); ++x) {
            int sum = 0;
            for (int i = -blurRadius; i <= blurRadius; ++i) {
                if (x + i >= 0 && x + i < static_cast<int>(size.x)) {
                    sum += density[x + i];
                }
            }
            smoothed[x] = sum;
        }

        struct Island { int minX; int maxX; int centerX; };
        std::vector<Island> islands;
        bool inIsland = false;
        int currentStart = 0;

        for (int x = 0; x < static_cast<int>(size.x); ++x) {
            if (smoothed[x] > 0) {
                if (!inIsland) {
                    currentStart = x;
                    inIsland = true;
                }
            }
            else {
                if (inIsland) {
                    int trueMinX = currentStart;
                    int trueMaxX = x - 1;
                    while (trueMinX <= trueMaxX && density[trueMinX] == 0) trueMinX++;
                    while (trueMaxX >= trueMinX && density[trueMaxX] == 0) trueMaxX--;

                    if (trueMinX <= trueMaxX && (trueMaxX - trueMinX + 1 > 8)) {
                        islands.push_back({ trueMinX, trueMaxX, (trueMinX + trueMaxX) / 2 });
                    }
                    inIsland = false;
                }
            }
        }
        if (inIsland) {
            int trueMinX = currentStart;
            int trueMaxX = size.x - 1;
            while (trueMinX <= trueMaxX && density[trueMinX] == 0) trueMinX++;
            while (trueMaxX >= trueMinX && density[trueMaxX] == 0) trueMaxX--;
            if (trueMinX <= trueMaxX && (trueMaxX - trueMinX + 1 > 8)) {
                islands.push_back({ trueMinX, trueMaxX, (trueMinX + trueMaxX) / 2 });
            }
        }

        if (islands.empty()) return anim;

        int minY_global = -1;
        int maxY_global = -1;
        for (unsigned int y = 0; y < safeHeight; ++y) {
            bool rowHasPixels = false;
            for (const auto& island : islands) {
                for (int x = island.minX; x <= island.maxX; ++x) {
                    if (image.getPixel({ static_cast<unsigned int>(x), y }).a > 0) {
                        rowHasPixels = true;
                        break;
                    }
                }
                if (rowHasPixels) break;
            }
            if (rowHasPixels) {
                if (minY_global == -1) minY_global = y;
                maxY_global = y;
            }
        }

        if (minY_global == -1) minY_global = 0;
        if (maxY_global == -1) maxY_global = safeHeight - 1;
        int frameHeight = maxY_global - minY_global + 1;

        int maxIslandWidth = 0;
        for (const auto& island : islands) {
            int w = island.maxX - island.minX + 1;
            if (w > maxIslandWidth) maxIslandWidth = w;
        }

        int finalFrameWidth = maxIslandWidth + 4;

        for (const auto& island : islands) {
            int left = island.centerX - (finalFrameWidth / 2);
            if (left < 0) left = 0;
            if (left + finalFrameWidth > static_cast<int>(size.x)) {
                left = static_cast<int>(size.x) - finalFrameWidth;
            }

            sf::Vector2i rectPos(left, minY_global);
            sf::Vector2i rectSize(finalFrameWidth, frameHeight);
            anim.frames.emplace_back(rectPos, rectSize);
        }

        anim.frameDuration = totalDuration / static_cast<float>(anim.frames.size());
        return anim;
    }
};