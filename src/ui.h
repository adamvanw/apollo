#ifndef UI_H
#define UI_H

#include "includes.h"
using namespace std;

struct TimelineLayer {
    string name;
    vector<bool> frames; // true if frame exists, false if not yet created
};

// purely for testing, claude helped me out here
class AnimationTimeline {
private:
    vector<TimelineLayer> timelineLayers;
    int maxFrames;
    float frameButtonSize;
    float frameSpacing;
    ImVec4 activeFrameColor;
    ImVec4 selectedFrameColor;
    ImVec4 inactiveFrameColor;

public:
    AnimationTimeline() :
        maxFrames(1),
        frameButtonSize(30.0f),
        frameSpacing(5.0f),
        selectedFrameColor(ImVec4(0.5f, 0.5f, 0.8f, 1.0f)),
        activeFrameColor(ImVec4(0.2f, 0.7f, 0.2f, 1.0f)),
        inactiveFrameColor(ImVec4(0.5f, 0.5f, 0.5f, 0.5f)) {
        // Initialize with one empty layer
        addLayer("Layer 1");
    }

    void update(vector<Layer> *layers) {
        for (int i = 0; i < layers->size(); i++) {
            if (i >= timelineLayers.size()) {
                string newName = "Layer " + to_string(i + 1);
                this->addLayer(newName);
            }
            int maxFramesInLayer = layers->at(i).startPos;
            for (int j = 0; j < layers->at(i).frames.size(); j++) {
                maxFramesInLayer += layers->at(i).frames.at(j)->getLength();
            }
            if (this->maxFrames < maxFramesInLayer) this->maxFrames = maxFramesInLayer;
            timelineLayers.at(i).frames.resize(this->maxFrames, false);
            for (int j = 0; j < layers->at(i).frames.size(); j++) {
                // TODO: incorporate length
                timelineLayers.at(i).frames[layers->at(i).startPos + j] = true;
            }
        }
    }

    void addLayer(const std::string& name) {
        TimelineLayer layer;
        layer.name = name;
        layer.frames.resize(maxFrames, false);
        timelineLayers.push_back(layer);
    }

    void render(vector<Layer> *layers, unsigned selectedLayerNum, unsigned selectedFrameNum, unsigned selectedTimelineNum) {
        ImGui::Begin("Timeline");

        ImGui::PushStyleColor(ImGuiCol_Button, selectedFrameColor);
        ImGui::SameLine(108.5f + selectedTimelineNum * (frameButtonSize + frameSpacing) - (frameSpacing / 2));
        ImGui::Button("##TimelinePos", ImVec2(frameSpacing + frameButtonSize, 10.0f));
        ImGui::PopStyleColor();

        ImGui::BeginChild("Scrollable Timeline", ImVec2(0, 0), true, ImGuiWindowFlags_HorizontalScrollbar);

        for (int layerIdx = layers->size() - 1; layerIdx >= 0; layerIdx--) {
            Layer* layer = &layers->at(layerIdx);
            ImGui::Text("Layer %d", layerIdx + 1);
            unsigned x = layer->startPos;
            for (int i = 0; i < layer->frames.size(); i++) {
                if (layer->frames[i]->deleted) continue;
                ImGui::SameLine(100.0f + x * (frameButtonSize + frameSpacing));

                string buttonId = "##" + to_string(layerIdx) + "_" + to_string(i);

                if (i == selectedFrameNum && layerIdx == selectedLayerNum) ImGui::PushStyleColor(ImGuiCol_Button, selectedFrameColor);
                else ImGui::PushStyleColor(ImGuiCol_Button, activeFrameColor);

                ImGui::Button(buttonId.c_str(), ImVec2(frameButtonSize + ((layer->frames[i]->getLength() - 1) * (frameButtonSize + frameSpacing)), frameButtonSize));
                x += layer->frames[i]->getLength();

                ImGui::PopStyleColor();
            }

            ImGui::Dummy(ImVec2(0.0f, frameSpacing));
        }

        ImGui::EndChild();
        ImGui::End();
    }

    /*
    void render(vector<Layer>* layers, unsigned selectedLayerNum, unsigned selectedFrameNum) {
        ImGui::Begin("Animation Timeline");

        // Create a child window that can be scrolled horizontally
        ImGui::BeginChild("TimelineScroll", ImVec2(0, 0), true, ImGuiWindowFlags_HorizontalScrollbar);

        // Draw frame numbers
        ImGui::SetCursorPosX(100.0f); // Offset for layer names
        for (int frame = 0; frame < maxFrames; frame++) {
            ImGui::SameLine(100.0f + frame * (frameButtonSize + frameSpacing));
            ImGui::Text("%d", frame + 1);
        }

        ImGui::Dummy(ImVec2(0.0f, 5.0f)); // Spacing after frame numbers

        // Draw layers and frames
        for (size_t layerIdx = timelineLayers.size() - 1; layerIdx >= 0; layerIdx--) {
            auto& layer = timelineLayers[layerIdx];

            // Layer name
            ImGui::Text("%s", layer.name.c_str());

            // Draw frames for this layer
            for (int frame = 0; frame < maxFrames; frame++) {
                ImGui::SameLine(100.0f + frame * (frameButtonSize + frameSpacing));

                // Create unique ID for each button
                std::string buttonId = "##" + std::to_string(layerIdx) + "_" + std::to_string(frame);

                // Set button color based on frame state
                if (layer.frames[frame]) {
                    if (layerIdx == selectedLayerNum && frame == selectedFrameNum + layers->at(selectedLayerNum).startPos) {
                        ImGui::PushStyleColor(ImGuiCol_Button, selectedFrameColor);
                    } else {
                        ImGui::PushStyleColor(ImGuiCol_Button, activeFrameColor);
                    }
                } else {
                    ImGui::PushStyleColor(ImGuiCol_Button, inactiveFrameColor);
                }


                if (ImGui::Button(buttonId.c_str(), ImVec2(frameButtonSize + ((layers->at(layerIdx).frames.size() > frame) ? (layers->at(layerIdx).frames[frame]->getLength() - 1) : 0) * (frameButtonSize + frameSpacing), frameButtonSize))) {
                    layer.frames[frame] = !layer.frames[frame]; // Toggle frame state
                }
                ImGui::PopStyleColor();
            }

            ImGui::Dummy(ImVec2(0.0f, frameSpacing)); // Spacing between layers
            if (layerIdx == 0) break;
        }

        // "Add Layer" button at the bottom
        if (ImGui::Button("Add Layer")) {
            addLayer("Layer " + std::to_string(timelineLayers.size() + 1));
        }

        // "Add Frame" button to extend timeline
        ImGui::SameLine();
        if (ImGui::Button("Add Frame")) {
            maxFrames++;
            for (auto& layer : timelineLayers) {
                layer.frames.push_back(false);
            }
        }

        ImGui::EndChild();
        ImGui::End();
    }
    */
};



#endif //UI_H
