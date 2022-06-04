@GUI::Frame {
    fill_with_background_color: true
    layout: @GUI::VerticalBoxLayout {
        margins: [10]
    }

    @GUI::GroupBox {
        title: "Highlight color"

        layout: @GUI::VerticalBoxLayout {
            margins: [6]
            spacing: 2
        }

        @GUI::ColorInput {
            name: "highlight_color_input"
        }

        fixed_height: 80
    }

    @GUI::GroupBox {
        title: "Highlight opacity"

        fixed_height: 80

       layout: @GUI::VerticalBoxLayout {
            margins: [6]
            spacing: 2
        }

        @GUI::Widget {
            layout: @GUI::HorizontalBoxLayout {
                margins: [8]
                spacing: 8
            }

            @GUI::Label {
                autosize: true
                text: "0%"
            }

            @GUI::Slider {
                name: "highlight_opacity_slider"
                orientation: "Horizontal"
                max: 100
                min: 0
                value: 30
            }

            @GUI::Label {
                autosize: true
                text: "100%"
            }
        }
    }

    @GUI::GroupBox {
        title: "Highlight radius"

        fixed_height: 80

       layout: @GUI::VerticalBoxLayout {
            margins: [6]
            spacing: 2
        }

        @GUI::Widget {
            layout: @GUI::HorizontalBoxLayout {
                margins: [8]
                spacing: 8
            }

            @GUI::Label {
                autosize: true
                text: "Smallest"
            }

            @GUI::Slider {
                name: "highlight_radius_slider"
                orientation: "Horizontal"
                max: 60
                min: 20
                value: 30
            }

            @GUI::Label {
                autosize: true
                text: "Largest"
            }
        }
    }
}
