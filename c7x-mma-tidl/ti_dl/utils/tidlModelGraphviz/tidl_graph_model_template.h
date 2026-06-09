#ifndef TIDL_GRAPH_MODEL_TEMPLATE_
#define TIDL_GRAPH_MODEL_TEMPLATE_

const char* HTML_TEMPLATE = R"raw(
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
</head>
<body>

    <div id="mainSvg">
    </div>

    <div id="search-icon" title="Search for nodes (Ctrl+F)">
        <svg xmlns="http://www.w3.org/2000/svg" width="24" height="24" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round">
            <circle cx="11" cy="11" r="8"></circle>
            <line x1="21" y1="21" x2="16.65" y2="16.65"></line>
        </svg>
    </div>
    <div id="search-container" style="display: none;">
        <input type="text" id="search-input">
        <button id="search-button">Search</button>
        <button id="search-close">✕</button>
    </div>
    <div id="download-icon" onclick="downloadSVG()" title="Download SVG">
        <svg xmlns="http://www.w3.org/2000/svg" width="24" height="24" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round">
            <line x1="12" y1="5" x2="12" y2="16"></line>
            <line x1="7" y1="12" x2="12" y2="16"></line>
            <line x1="12" y1="16" x2="17" y2="12"></line>
            <line x1="22" y1="15" x2="22" y2="22"></line>
            <line x1="2" y1="15" x2="2" y2="22"></line>
            <line x1="2" y1="22" x2="22" y2="22"></line>
        </svg>
    </div>
    <div id="zoom-in-icon" onclick="zoomIn()" title="Zoom In">
        <svg xmlns="http://www.w3.org/2000/svg" width="24" height="24" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round">
            <circle cx="11" cy="11" r="8"></circle>
            <line x1="21" y1="21" x2="16.65" y2="16.65"></line>
            <line x1="11" y1="8" x2="11" y2="14"></line>
            <line x1="8" y1="11" x2="14" y2="11"></line>
        </svg>
    </div>
    <div id="zoom-out-icon" onclick="zoomOut()" title="Zoom Out">
        <svg xmlns="http://www.w3.org/2000/svg" width="24" height="24" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round">
            <circle cx="11" cy="11" r="8"></circle>
            <line x1="21" y1="21" x2="16.65" y2="16.65"></line>
            <line x1="8" y1="11" x2="14" y2="11"></line>
        </svg>
    </div>
    <div id="sidenav" class="sidenav">
        <a href="javascript:void(0)" class="closebtn" onclick="closeSideBar()">&#215;</a>
        <div id="node_properties_container">
            <div class="properties-section" id="node_properties">
                <h3>NODE PROPERTIES</h3>
                <div class="property-row">
                    <label>layerNum</label>
                    <div class="property-value" id="node_layer_num"></div>
                </div>
                <div class="property-row">
                    <label>type</label>
                    <div class="property-value" id="node_type"></div>
                </div>
                <div class="property-row">
                    <label>name</label>
                    <div class="property-value-container">
                        <div class="property-value path-value" id="node_name"></div>
                        <div class="copy-button" onclick="copyNodeName()" title="Special Copy">
                            <svg xmlns="http://www.w3.org/2000/svg" width="16" height="16" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round">
                                <rect x="9" y="9" width="13" height="13" rx="2" ry="2"></rect>
                                <path d="M5 15H4a2 2 0 0 1-2-2V4a2 2 0 0 1 2-2h9a2 2 0 0 1 2 2v1"></path>
                            </svg>
                        </div>
                    </div>
                </div>
            </div>
            <div class="properties-section" id="inputs">
                <h3>INPUTS</h3>
            </div>
            <div class="properties-section" id="outputs">
                <h3>OUTPUTS</h3>
            </div>
            <div class="properties-section" id="attributes">
                <h3>ATTRIBUTES</h3>
            </div>
        </div>
    </div>

    <style>
        #download-icon {
            position: fixed;
            bottom: 5px;
            left: 5px;
            z-index: 100;
            padding: 5px;
            cursor: pointer;
        }
        
        #download-icon svg {
            width: 24px;
            height: 24px;
            stroke: #888888;
            transition: stroke 0.2s ease;
        }
        
        #download-icon:hover svg {
            stroke: #444444;
        }

        #zoom-in-icon {
            position: fixed;
            bottom: 5px;
            left: 45px;
            z-index: 100;
            padding: 5px;
            cursor: pointer;
        }
        
        #zoom-in-icon svg {
            width: 24px;
            height: 24px;
            stroke: #888888;
            transition: stroke 0.2s ease;
        }
        
        #zoom-in-icon:hover svg {
            stroke: #444444;
        }

        #zoom-out-icon {
            position: fixed;
            bottom: 5px;
            left: 85px;
            z-index: 100;
            padding: 5px;
            cursor: pointer;
        }
        
        #zoom-out-icon svg {
            width: 24px;
            height: 24px;
            stroke: #888888;
            transition: stroke 0.2s ease;
        }
        
        #zoom-out-icon:hover svg {
            stroke: #444444;
        }

        #search-icon {
            position: fixed;
            top: 5px;
            left: 5px;
            z-index: 100;
            padding: 5px;
            cursor: pointer;
        }
        
        #search-icon svg {
            width: 24px;
            height: 24px;
            stroke: #888888;
            transition: stroke 0.2s ease;
        }
        
        #search-icon:hover svg {
            stroke: #444444;
        }
        
        #search-container {
            position: fixed;
            top: 5px;
            left: 5px;
            z-index: 100;
            background-color: rgba(255, 255, 255, 0.8);
            padding: 5px;
            border-radius: 5px;
            box-shadow: 0 2px 5px rgba(0, 0, 0, 0.2);
            display: flex;
            align-items: center;
        }
        
        #search-input {
            padding: 5px 10px;
            border: 1px solid #ccc;
            border-radius: 3px;
            margin-right: 5px;
            width: 200px;
        }
        
        #search-button {
            padding: 5px 10px;
            background-color: #4CAF50;
            color: white;
            border: none;
            border-radius: 3px;
            cursor: pointer;
            margin-right: 5px;
        }
        
        #search-button:hover {
            background-color: #45a049;
        }
        
        #search-close {
            padding: 5px 8px;
            background-color: #f44336;
            color: white;
            border: none;
            border-radius: 3px;
            cursor: pointer;
            font-size: 12px;
        }
        
        #search-close:hover {
            background-color: #d32f2f;
        }
        
        .nodeHighlight {
            stroke: #ff0000 !important;
            stroke-width: 3px !important;
        }
        
        .sidenav {
            height: 100%; /* 100% Full-height */
            width: 0%; /* 0 width - change this with JavaScript */
            position: fixed; /* Stay in place */
            z-index: 1; /* Stay on top */
            top: 0; /* Stay at the top */
            right: 0;
            background-color: #eaeaea;
            color: black;
            overflow-x: hidden; /* Disable horizontal scroll */
            transition: 0.3s; /* 0.5 second transition effect to slide in the sidenav */
            padding: 0px;
            box-sizing: border-box;
        }
        
        /* When you mouse over the navigation links, change their color */
        .sidenav a:hover {
            color: #f1f1f1;
        }

        .sidenav .closebtn {
                position: absolute;
                top: 2px;
                right: 10px;
                background: none;
                border: none;
                color: #9c9c9c;
                font-size:35px;
                cursor: pointer;
                text-decoration: none;
            }

        .sidenav .closebtn:hover {
            color: #3c3c3c;
        }
        
        /* Node properties styling */
        #node_properties_container {
            font-family: Arial, sans-serif;
            font-size:14px;
            margin-top: 10px;
        }
        
        .properties-section {
            margin-bottom: 20px;
        }
        
        .properties-section h3 {
            font-size:16px;
            font-weight: bold;
            margin-bottom: 10px;
            color: #333;
            cursor: pointer;
            user-select: none;
            display: flex;
            justify-content: space-between;
            align-items: center;
        }
        
        .properties-section h3::after {
            content: "▼";
            font-size:12px;
            transition: transform 0.3s;
        }
        
        .properties-section.collapsed h3::after {
            transform: rotate(-90deg);
        }
        
        .properties-section.collapsed .property-row {
            display: none;
        }
        
        .property-row {
            display: flex;
            margin-bottom: 8px;
            align-items: flex-start;
            width: 100%;
            min-width: 0;
            box-sizing: border-box;
        }
        
        .property-row label {
            width:30%;
            text-align: right;
            padding-right: 10px;
            color: #555;
            overflow: hidden;
            text-overflow: ellipsis;
            white-space: nowrap;
            box-sizing: border-box;
        }
        
        .property-value-container {
            width: 70%;
            display: flex;
            align-items: center;
        }
        
        .property-value {
            flex-grow: 1;
            word-break: break-all;
            padding: 2px 5px;
            background-color: #f5f5f5;
            border-radius: 3px;
            max-height: 60px;
            overflow-y: auto;
            scrollbar-width: thin;
            scrollbar-color: #999 #f5f5f5;
            white-space: nowrap;
            position: relative;
            box-sizing: border-box;
        }
        
        .copy-button {
            margin-left: 5px;
            cursor: pointer;
            padding: 2px;
            border-radius: 3px;
            display: flex;
            align-items: center;
            justify-content: center;
        }
        
        .copy-button:hover {
            background-color: #e0e0e0;
        }
        
        .copy-button svg {
            width: 16px;
            height: 16px;
            stroke: #888888;
        }
        
        .copy-button:hover svg {
            stroke: #444444;
        }
        
        /* For WebKit browsers (Chrome, Safari) */
        .property-value::-webkit-scrollbar {
            width: 6px;
            height: 6px;
        }

        .property-value::-webkit-scrollbar-track {
            background: #f5f5f5;
        }

        .property-value::-webkit-scrollbar-thumb {
            background-color: #999;
            border-radius: 3px;
        }
    </style>

    <script>
        var currentSelectedNodeId = null;
        const nodeInfoMap = new Map();
        const edgeInfoMap = new Map();

         // Variable to track current zoom level
        let currentZoom = 1.0;
        
        function zoomIn() {
            currentZoom += 0.1;
            applyZoom();
        }
        
        function zoomOut() {
            if (currentZoom > 0.2) {
                currentZoom -= 0.1;
                applyZoom();
            }
        }
        
        function applyZoom() {
            const mainSvg = document.getElementById("mainSvg").querySelector('svg');
            if (!mainSvg) {
                return;
            }
            
            mainSvg.style.transform = `scale(${currentZoom})`;
            mainSvg.style.transformOrigin = 'center center';
        }
        
        function copyNodeName() {
            const nodeName = document.getElementById('node_name').textContent;
            if (!nodeName) return;
            
            // Create a temporary textarea to copy the text
            const textarea = document.createElement('textarea');
            textarea.value = nodeName.trim().replace(/\//g, '_');
            document.body.appendChild(textarea);
            textarea.select();
            
            try {
                const successful = document.execCommand('copy');
                if (successful) {
                    // Show a temporary success message
                    const message = document.createElement('div');
                    message.textContent = 'Copied to clipboard!';
                    message.style.position = 'fixed';
                    message.style.top = '10px';
                    message.style.right = '10px';
                    message.style.backgroundColor = 'rgba(0, 0, 0, 0.7)';
                    message.style.color = 'white';
                    message.style.padding = '8px 12px';
                    message.style.borderRadius = '4px';
                    message.style.zIndex = '1000';
                    document.body.appendChild(message);
                    
                    // Remove the message after 2 seconds
                    setTimeout(() => {
                        document.body.removeChild(message);
                    }, 2000);
                }
            } catch (err) {
                console.error('Failed to copy node name: ', err);
            }
            
            document.body.removeChild(textarea);
        }
        
        function downloadSVG() {
            const mainSvg = document.getElementById("mainSvg").querySelector('svg');
            if (!mainSvg) {
                return;
            }

            let title = mainSvg.querySelector('title').textContent;
            const lastBackslashIndex = title.lastIndexOf('\\');
            const lastSlashIndex = title.lastIndexOf('/');
            const lastIndex = Math.max(lastBackslashIndex, lastSlashIndex);
            if (lastIndex != -1) {
                title = title.substring(lastIndex + 1);
            }
            title = title + ".svg";

            const serializer = new XMLSerializer();
            let svgString = serializer.serializeToString(mainSvg);
            svgString = svgString.replace(/ /g, " ");
            const blob = new Blob([svgString], { type: 'image/svg+xml' });
            const a = document.createElement('a');
            a.download = title;
            a.href = window.URL.createObjectURL(blob);
            document.body.appendChild(a);
            a.click();
            document.body.removeChild(a);
            window.URL.revokeObjectURL(a.href);
        }

        // Function to calculate the maximum label width and adjust the CSS accordingly
        function setSideBarValues(nodeId) {
            const sidenav = document.getElementById('sidenav');
            const node = document.getElementById(nodeId);
            const nodeContent = node.querySelector("title").textContent;
            const splitNodeContent = nodeContent.split(/\n+/).filter(word => word !== '');
            const splitNodeContentTrim = splitNodeContent.map(str => str.trim());
            
            // Add node information
            const nodeInfo = splitNodeContentTrim.shift().split(/\s+/).filter(word => word !== '');
            document.getElementById('node_layer_num').textContent = nodeInfo[1].replace(/\D/g, '');
            document.getElementById('node_type').textContent = nodeInfo[2];
            document.getElementById('node_name').textContent = nodeInfo[3].trim().replace(/^['"]|['"]$/g, '');

            // Clear previous inputs
            const inputsDiv = document.getElementById('inputs');
            while (inputsDiv.children.length > 1) {
                inputsDiv.removeChild(inputsDiv.lastChild);
            }
            
            // Clear previous outputs
            const outputsDiv = document.getElementById('outputs');
            while (outputsDiv.children.length > 1) {
                outputsDiv.removeChild(outputsDiv.lastChild);
            }

            // Clear previous attributes
             const attributesDiv = document.getElementById('attributes');
            while (attributesDiv.children.length > 1) {
                attributesDiv.removeChild(attributesDiv.lastChild);
            }
            

            // Add input/output information
            const ioHtmlTemplate = `
                <div class="property-row">
                    <label class="property-label"></label>
                    <div class="property-value" nodeId=""></div>
                </div>
            `;

            var inputNodes = new Array();
            edgeInfoMap.forEach((value, key) => {
                if (value.get("output_node") === nodeId) {
                    inputNodes.push(value.get("input_node"));
                }
            });

            var outputNodes = new Array();
            edgeInfoMap.forEach((value, key) => {
                if (value.get("input_node") === nodeId) {
                    outputNodes.push(value.get("output_node"));
                }
            });

            const ioMap = new Map([['inputs', inputNodes], ['outputs', outputNodes]]);
            ioMap.forEach((value, key) => {

                var div = null;
                if (key === 'inputs') {
                    div = inputsDiv;
                    
                } else {
                    div = outputsDiv
                }

                const divHeading = div.querySelector('h3');

                if (value.length < 1) {
                    divHeading.style.display = 'none';
                } else {
                    divHeading.style.display = 'flex';
                    i = 0;
                    while (i < value.length) {
                        const inputNode = nodeInfoMap.get(value[i]);
                        const newInputDiv = document.createElement('div');
                        newInputDiv.innerHTML = ioHtmlTemplate;
                        const labelElement = newInputDiv.querySelector(".property-label");
                        const valueElement = newInputDiv.querySelector(".property-value");

                        labelElement.textContent = inputNode.get("layer");

                        valueElement.textContent = inputNode.get("name").trim().replace(/^['"]|['"]$/g, '');
                        valueElement.setAttribute("nodeId", value[i]);
                        valueElement.style.cursor = 'pointer';
                        newInputDiv.querySelector(".property-value").addEventListener('click', function(event) {
                            const clickEvent = new Event('click');
                            document.getElementById(event.target.getAttribute('nodeId')).dispatchEvent(clickEvent);
                        });
                        div.appendChild(newInputDiv);
                        i++;
                    }
                }
            });

            // Add attribute information
            const attributeHtmlTemplate = `
                <div class="property-row">
                    <label class="property-label" title=""></label>
                    <div class="property-value"></div>
                </div>
            `;

            var attributeInfo;
            var parts;

            var i = 0;
            let splitNodeContentTrimCorrected = new Array();
            while (i < splitNodeContentTrim.length) {
                if (!splitNodeContentTrim[i].endsWith(":")) {
                    splitNodeContentTrimCorrected.push(splitNodeContentTrim[i]);
                } else {
                    const lastSpaceIndex = splitNodeContentTrim[i].lastIndexOf(' ');
                    if (lastSpaceIndex !== -1)
                    {
                        splitNodeContentTrimCorrected.push(splitNodeContentTrim[i].substring(0, lastSpaceIndex).trim());
                        splitNodeContentTrimCorrected.push(splitNodeContentTrim[i].substring(lastSpaceIndex + 1).trim());
                    } else {
                        splitNodeContentTrimCorrected.push(splitNodeContentTrim[i].trim());
                    }
                }
                i++;
            }

            let tempDataArray1 = new Array();
            let tempDataArray2 = new Array();
            let lineParsed = new Array();

            i = 0;
            while (i < splitNodeContentTrimCorrected.length) {
                if (splitNodeContentTrimCorrected[i].endsWith(":") && !lineParsed.includes(i)) {
                    let attrPrefix = splitNodeContentTrimCorrected[i];
                    const lastSpaceIndex = splitNodeContentTrimCorrected[i].lastIndexOf(' ');
                    if (lastSpaceIndex !== -1) {
                        attrPrefix = splitNodeContentTrimCorrected[i].substring(lastSpaceIndex + 1);
                    }

                    for (let j = i+1; j < splitNodeContentTrimCorrected.length; j++) {
                        if (splitNodeContentTrimCorrected[j].endsWith(":")) {
                            i--;
                            break;
                        }

                        attributeInfo = splitNodeContentTrimCorrected[j].split(/\s+/).filter(word => word !== '');

                        for (let k = 0; k < attributeInfo.length; k++) {
                            if (attributeInfo[k].includes('=')) {
                                parts = attributeInfo[k].split(/=(.*)/);
                            } else if (attributeInfo[k].includes(':')) {
                                parts = attributeInfo[k].split(/:(.*)/);
                            } else {
                                continue;
                            }
                            let key = attrPrefix + parts[0]
                            let val = parts[1];
                            if (val && val.endsWith(',')) {
                                val = val.slice(0, -1);
                            }
                            tempDataArray2.push([key,val])
                        }

                        lineParsed.push(j);
                        i++;
                    }
                    lineParsed.push(i);
                }
                i++;
            }

            
            i = 0;
            while (i < splitNodeContentTrimCorrected.length) {
                if (!splitNodeContentTrimCorrected[i].endsWith(":") && !lineParsed.includes(i))
                {
                    const attributeInfo = splitNodeContentTrimCorrected[i].split(/\s+/).filter(word => word !== '');
                    for (let j = 0; j < attributeInfo.length; j++) {
                        var parts;
                        if (attributeInfo[j].includes('=')) {
                            parts = attributeInfo[j].split(/=(.*)/);
                        } else if (attributeInfo[j].includes(':')) {
                            parts = attributeInfo[j].split(/:(.*)/);
                        } else {
                            continue;
                        }
                        let key = parts[0]
                        let val = parts[1];
                        if (val && val.endsWith(',')) {
                            val = val.slice(0, -1);
                        }
                        tempDataArray1.push([key,val])
                    }
                    lineParsed.push(i);
                }
                i++;
            }

            const dataArray = tempDataArray1.concat(tempDataArray2);

            for (const element of dataArray) {
                const newAttributeDiv = document.createElement('div');
                newAttributeDiv.innerHTML = attributeHtmlTemplate;
                const labelElement = newAttributeDiv.querySelector(".property-label");
                const valueElement = newAttributeDiv.querySelector(".property-value");
                labelElement.textContent = element[0];
                labelElement.title = element[0];
                valueElement.textContent = element[1];
                attributesDiv.appendChild(newAttributeDiv);
            }
        }

        function openSideBar(nodeId) {
            const sidenav = document.getElementById('sidenav');
            sidenav.style.width = "35%";
            sidenav.style.padding = "20px";
            setSideBarValues(nodeId);
        }

        function closeSideBar(nodeId) {
            const sidenav = document.getElementById('sidenav');
            sidenav.style.width = "0px";
            sidenav.style.padding = "0px";
        }

        function highlightCurrentNode(nodeId) {
            if (currentSelectedNodeId != nodeId)
            {
                if (currentSelectedNodeId)
                {
                    const currNode =  document.getElementById(currentSelectedNodeId);
                    currNode.querySelector('path').classList.remove('nodeHighlight');
                }
                const currNode =  document.getElementById(nodeId);
                currNode.querySelector('path').classList.add('nodeHighlight');
            }
        }

        function scrollToNode(nodeId) {
            const sidenav = document.getElementById('sidenav');
            const node =  document.getElementById(nodeId);
            const rect = node.getBoundingClientRect();
            const viewportWidth = window.innerWidth || document.documentElement.clientWidth;
            const viewportHeight = window.innerHeight || document.documentElement.clientHeight;
            const targetX = rect.left + (rect.width / 2) - ((viewportWidth - sidenav.offsetWidth) / 2);
            const targetY = rect.top + (rect.height / 2) - (viewportHeight / 2);
            window.scrollBy({
                top: targetY,
                left: targetX,
                behavior: 'smooth'
            });
        }

        // Function to toggle section collapse
        function toggleSection(section) {
            section.classList.toggle('collapsed');
        }
        
        function searchNode() {
            const searchInput = document.getElementById('search-input').value.trim();
            if (!searchInput) return;

            // Search for node with matching ID or containing the search term
            let found = false;

            const foundLayerNum = new Array();
            const foundLayerName = new Array();

            nodeInfoMap.forEach((value, key) => {
                const layerNum = value.get("layer");
                const layerName = value.get("name");
                const layerOpType = value.get("op_type");
                if (layerNum.toLowerCase().includes(searchInput.toLowerCase())) {
                    foundLayerNum.push([layerNum, key]);
                }
                if (layerName.toLowerCase().includes(searchInput.toLowerCase())) {
                    foundLayerName.push([layerName, key]);
                }
                if (layerOpType.trim().split(' ')[0].toLowerCase().includes(searchInput.toLowerCase())) {
                    foundLayerName.push([layerOpType, key]);
                }
            });

            let foundNode = null;
            if (foundLayerNum.length == 1 && foundLayerName.length < 1) {
                foundNode = document.getElementById(foundLayerNum[0][1]);
            } else if (foundLayerNum.length < 1 && foundLayerName.length == 1){
                foundNode = document.getElementById(foundLayerName[0][1]);
            } else {
                showSearchDropdown(foundLayerNum, foundLayerName);
                return;
            }

            if (foundNode) {
                scrollToNode(foundNode.id);
                openSideBar(foundNode.id);
                highlightCurrentNode(foundNode.id);
                currentSelectedNodeId = foundNode.id;
            } else {
                alert('No element found with query: ' + searchInput);
            }
        }
        
        function showSearchDropdown(foundLayerNum, foundLayerName) {
            // Remove existing dropdown if any
            const existingDropdown = document.getElementById('search-dropdown');
            if (existingDropdown) {
                existingDropdown.remove();
            }
            
            // If no results found
            if (foundLayerNum.length === 0 && foundLayerName.length === 0) {
                alert('No element found with the search query');
                return;
            }
            
            // Create dropdown
            const dropdown = document.createElement('div');
            dropdown.id = 'search-dropdown';
            dropdown.style.position = 'fixed';
            dropdown.style.top = (document.getElementById('search-container').getBoundingClientRect().bottom + 5) + 'px';
            dropdown.style.left = document.getElementById('search-container').getBoundingClientRect().left + 'px';
            dropdown.style.width = '300px';
            dropdown.style.maxHeight = '300px';
            dropdown.style.overflowY = 'auto';
            dropdown.style.backgroundColor = 'white';
            dropdown.style.border = '1px solid #ccc';
            dropdown.style.borderRadius = '3px';
            dropdown.style.boxShadow = '0 2px 5px rgba(0, 0, 0, 0.2)';
            dropdown.style.zIndex = '1000';
            dropdown.style.display = 'block'; // Ensure the dropdown is visible
            
            // Add header for layer numbers if any found
            if (foundLayerNum.length > 0) {
                const layerNumHeader = document.createElement('div');
                layerNumHeader.style.padding = '5px 10px';
                layerNumHeader.style.fontWeight = 'bold';
                layerNumHeader.style.backgroundColor = '#f0f0f0';
                layerNumHeader.textContent = 'Matching Layer Numbers:';
                dropdown.appendChild(layerNumHeader);
                
                // Add layer number results
                foundLayerNum.forEach(item => {
                    const option = document.createElement('div');
                    option.style.padding = '5px 10px';
                    option.style.cursor = 'pointer';
                    option.textContent = item[0];
                    option.dataset.nodeId = item[1];
                    option.addEventListener('mouseover', function() {
                        this.style.backgroundColor = '#f0f0f0';
                    });
                    option.addEventListener('mouseout', function() {
                        this.style.backgroundColor = 'transparent';
                    });
                    option.addEventListener('click', function() {
                        const nodeId = this.dataset.nodeId;
                        const node = document.getElementById(nodeId);
                        if (node) {
                            scrollToNode(nodeId);
                            openSideBar(nodeId);
                            highlightCurrentNode(nodeId);
                            currentSelectedNodeId = nodeId;
                            //dropdown.remove();
                        }
                    });
                    dropdown.appendChild(option);
                });
            }
            
            // Add header for layer names if any found
            if (foundLayerName.length > 0) {
                const layerNameHeader = document.createElement('div');
                layerNameHeader.style.padding = '5px 10px';
                layerNameHeader.style.fontWeight = 'bold';
                layerNameHeader.style.backgroundColor = '#f0f0f0';
                layerNameHeader.textContent = 'Matching Layer Names:';
                dropdown.appendChild(layerNameHeader);
                
                // Add layer name results
                foundLayerName.forEach(item => {
                    const option = document.createElement('div');
                    option.style.padding = '5px 10px';
                    option.style.cursor = 'pointer';
                    option.textContent = item[0];
                    option.dataset.nodeId = item[1];
                    option.addEventListener('mouseover', function() {
                        this.style.backgroundColor = '#f0f0f0';
                    });
                    option.addEventListener('mouseout', function() {
                        this.style.backgroundColor = 'transparent';
                    });
                    option.addEventListener('click', function() {
                        const nodeId = this.dataset.nodeId;
                        const node = document.getElementById(nodeId);
                        if (node) {
                            scrollToNode(nodeId);
                            openSideBar(nodeId);
                            highlightCurrentNode(nodeId);
                            currentSelectedNodeId = nodeId;
                            //dropdown.remove();
                        }
                    });
                    dropdown.appendChild(option);
                });
            }
            
            // Add dropdown to document
            document.body.appendChild(dropdown);
            
            // Close dropdown when clicking outside
            // document.addEventListener('click', function closeDropdown(e) {
            //     if (!dropdown.contains(e.target) && e.target.id !== 'search-input' && e.target.id !== 'search-button') {
            //         dropdown.remove();
            //         document.removeEventListener('click', closeDropdown);
            //     }
            // });
        }
        
        document.addEventListener('DOMContentLoaded', () => {
            // Set up search functionality
            document.getElementById('search-button').addEventListener('click', searchNode);
            document.getElementById('search-input').addEventListener('keypress', function(e) {
                if (e.key === 'Enter') {
                    searchNode();
                }
            });
            
            // Show search bar when search icon is clicked
            document.getElementById('search-icon').addEventListener('click', function() {
                document.getElementById('search-icon').style.display = 'none';
                document.getElementById('search-container').style.display = 'flex';
                document.getElementById('search-input').focus();
            });
            
            // Hide search bar when close button is clicked
            document.getElementById('search-close').addEventListener('click', function() {
                document.getElementById('search-container').style.display = 'none';
                document.getElementById('search-icon').style.display = 'block';
                const existingDropdown = document.getElementById('search-dropdown');
                if (existingDropdown) {
                    existingDropdown.remove();
                }
            });
            
            // Show search bar when Ctrl+F is pressed
            document.addEventListener('keydown', function(e) {
                if ((e.ctrlKey || e.metaKey) && e.key === 'f') {
                    e.preventDefault(); // Prevent the browser's default search
                    document.getElementById('search-icon').style.display = 'none';
                    document.getElementById('search-container').style.display = 'flex';
                    document.getElementById('search-input').focus();
                }
                
                // Hide search bar when Escape is pressed
                if (e.key === 'Escape' && document.getElementById('search-container').style.display !== 'none') {
                    document.getElementById('search-container').style.display = 'none';
                    document.getElementById('search-icon').style.display = 'block';
                    const existingDropdown = document.getElementById('search-dropdown');
                    if (existingDropdown) {
                        existingDropdown.remove();
                    }
                }
            });
            
            // Set up collapsible sections
            const sections = document.querySelectorAll('.properties-section');
            sections.forEach(section => {
                const header = section.querySelector('h3');
                header.addEventListener('click', () => {
                    toggleSection(section);
                });
            });
            
            var nodes = document.querySelectorAll('.node');
            var edges = document.querySelectorAll('.edge');

            nodes.forEach(function(node) {
                const infoMap = new Map([['layer', null], ['name', null], ['op_type', null], ['input_edges', new Array()], ['output_edges', new Array()]]);
                var nodeTitle = node.querySelector('title');
                if (nodeTitle) {
                    const nodeTitleTextContent = nodeTitle.textContent.trim();
                    const nodeColonIndex = nodeTitleTextContent.indexOf(':');
                    var nodeLayer = nodeTitleTextContent;
                    var nodeName = nodeTitleTextContent;
                    if (nodeTitleTextContent !== -1) {
                        nodeLayer = nodeTitleTextContent.substring(0, nodeColonIndex);
                        nodeName = nodeTitleTextContent.substring(nodeColonIndex + 1).trim();
                        nodeName = nodeName.split(/\s+/).filter(word => word !== '')[1];
                    }
                    infoMap.set('layer', nodeLayer);
                    infoMap.set('name', nodeName);

                    var nodeText = node.querySelector('text');
                    const nodeTextContent = nodeText.textContent.trim();
                    var nodeOpType = nodeTextContent;
                    infoMap.set('op_type', nodeOpType);
                    
                    edges.forEach(function(edge) {                
                        var edgeTitle = edge.querySelector('title');
                        if (edgeTitle) {
                            const edgeTextContent = edgeTitle.textContent.trim();
                            const edgeColonIndex = edgeTextContent.indexOf(':');
                            var edgeLayer = edgeTextContent;
                            if (edgeTextContent !== -1) {
                                edgeLayer = edgeTextContent.substring(0, edgeColonIndex);
                            }
                            if (nodeLayer == edgeLayer) {
                                infoMap.get('output_edges').push(edge.id); 
                            }
                        }
                    });
                }
                nodeInfoMap.set(node.id, infoMap);
            });

            edges.forEach(function(edge) {                
                const infoMap = new Map([['input_node', null], ['output_node', null]]);
                var edgeTitle = edge.querySelector('title');
                if (edgeTitle) {
                    const edgeTextContent = edgeTitle.textContent.trim();
                    const edgeColonIndex = edgeTextContent.indexOf(':');
                    var edgeLayer = edgeTextContent;
                    if (edgeTextContent !== -1) {
                        edgeLayer = edgeTextContent.substring(0, edgeColonIndex);
                    }

                    nodeInfoMap.forEach((value, key) => {
                        const outputEdges = value.get("output_edges");
                        if (outputEdges.includes(edge.id)) {
                            infoMap.set('input_node', key);
                        }
                    });

                    const outNodeRe = edgeTextContent.match(/->Layer\s+(\d+):\s+(.*)/)
                    if (outNodeRe && outNodeRe[1]) {
                        const outLayer = "Layer " + outNodeRe[1];

                        nodeInfoMap.forEach((value, key) => {
                        const layer = value.get("layer");
                            if (outLayer === layer) {
                                infoMap.set('output_node', key);
                            }
                        });
                    }
                }
                edgeInfoMap.set(edge.id, infoMap);
            });

            nodes.forEach(function(node) {
                node.style.cursor = 'pointer';
                node.addEventListener('click', () => {
                    scrollToNode(node.id);
                    openSideBar(node.id);
                    highlightCurrentNode(node.id);
                    currentSelectedNodeId = node.id;
                });
            });

            edges.forEach(function(edge) {                
    
                edge.style.cursor = 'pointer';
                const path = edge.querySelector("path");
                const polygon = edge.querySelector("polygon");
                edge.addEventListener('mouseover', () => {
                    path.setAttribute("stroke", "red");
                    polygon.setAttribute("fill", "red");
                    polygon.setAttribute("stroke", "red");
                });
                edge.addEventListener('mouseout', () => {
                    path.setAttribute("stroke", "black");
                    polygon.setAttribute("fill", "black");
                    polygon.setAttribute("stroke", "black");
                });
                edge.addEventListener('click', () => {
                    const parentNode = document.getElementById(edgeInfoMap.get(edge.id).get('input_node'));
                    if(parentNode) {
                        scrollToNode(parentNode.id);
                        openSideBar(parentNode.id);
                        highlightCurrentNode(parentNode.id);
                        currentSelectedNodeId = parentNode.id;
                    }
                });
            });

        });
    </script>
</body>
</html>
)raw";

#endif 