#include <SPIFFS.h>

const char* webtest_html = R"rawliteral(

<!DOCTYPE html>
<html>
<head>
    <title>Web Servo</title>
    <style>
        body {
            text-align: center;
            background-color: rgb(50, 0, 56);
            color: white;
        }
    </style>
</head>
<body>
    <input type="range" min="0" max="100" value="0" size="500" class="slider" id="slider1">
    <p id="slider1_output">0</p>
    <button type="button" id="btn1">fetch</button>    
    <p id="real_slider1_output">X</p>
<script>
    var last_slider1_value = 0;
    var slider1 = document.getElementById("slider1");
    const slider1_output = document.getElementById("slider1_output");
    const btn1 = document.getElementById("btn1");
    const real_slider1_output = document.getElementById("real_slider1_output");

    slider1.addEventListener("input", function() {
        setTimeout(() => {
            slider1_output.textContent = slider1.value;
            if (slider1.value != last_slider1_value) 
            {
                last_slider1_value = slider1.value;
                fetch_command_and_output("/num=" + slider1.value, real_slider1_output);
            }
        }, 100
    );
    });

    btn1.addEventListener("click", function() {
        fetch_command_and_output("/num=" + slider1.value, real_slider1_output);
    });

    async function fetch_command_and_output(command, p_output) {
        console.log("fetch_command_and_output: " + command);
        fetch(command)
        .then((response) => {
            console.log(`Received response: ${response.status}`);
            return response.text();
        })
        .then((data) => {
            console.log(`Received data: ${data}`)
            const json = JSON.parse(data);
            p_output.textContent = json.num;
        });
    }

</script>
</body>
</html>

)rawliteral";

void setup() {
    Serial.begin(115200);
    // Serial.println("Starting SPIFFS test");
    // if (!SPIFFS.begin()) {
    //     Serial.println("Failed to mount SPIFFS");
    //     return;
    // }

    // File file = SPIFFS.open("/webtest.html", "r");
    // if (!file) {
    //     Serial.println("Failed to open file");
    //     return;
    // }
    // file.close();

    Serial.print("File content: ");
    Serial.println(webtest_html);

}

void loop() {
    // Your code here
}