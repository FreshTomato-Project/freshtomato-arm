<!DOCTYPE html>
<!--
	Tomato GUI
	Copyright (C) 2006-2010 Jonathan Zarate
	http://www.polarcloud.com/tomato/

	For use with Tomato Firmware only.
	No part of this file may be used without permission.
-->
<html lang="en-GB">
<head>
<meta http-equiv="content-type" content="text/html;charset=utf-8">
<meta name="robots" content="noindex,nofollow">
<title>[<% ident(); %>] File Browser</title>
<link rel="stylesheet" type="text/css" href="tomato.css">
<% css(); %>

<!-- Correct FreshTomato pattern for injecting NVRAM variables -->
<script>
//	<% nvram("http_id"); %>
</script>
<script src="tomato.js"></script>

<script>
var PathIntellisense = function(inputId) {
    this.input = document.getElementById(inputId);
    if (!this.input) return;
    
    this.container = null;
    this.list = null;
    this.suggestions = [];
    this.selectedIndex = -1;
    this.debounceTimer = null;
    this.directoryCache = {};
    this.xob = null;
    
    this.init();
};

PathIntellisense.prototype.init = function() {
    var self = this;
    
    this.container = document.createElement('div');
    this.container.style.cssText = 'position:absolute;top:100%;left:0;right:0;background:white;border:1px solid #ccc;border-top:none;max-height:300px;overflow-y:auto;z-index:9999;display:none;box-shadow:0 2px 5px rgba(0,0,0,0.1);';
    
    this.list = document.createElement('ul');
    this.list.style.cssText = 'list-style:none;margin:0;padding:0;';
    
    this.container.appendChild(this.list);
    this.input.parentNode.style.position = 'relative';
    this.input.parentNode.appendChild(this.container);
    
    this.input.addEventListener('input', function() {
        clearTimeout(self.debounceTimer);
        self.debounceTimer = setTimeout(function() {
            self.search(self.input.value);
        }, 300);
    });
    
    this.input.addEventListener('keydown', function(e) {
        self.handleKeydown(e);
    });
    
    this.input.addEventListener('blur', function() {
        setTimeout(function() { self.hide(); }, 200);
    });
};

PathIntellisense.prototype.fetchDirectory = function(dir, callback) {
    var self = this;
    
    if (this.directoryCache[dir]) {
        callback(this.directoryCache[dir]);
        return;
    }
    
    if (this.xob) return;
    
    this.xob = new XmlHttp();
    
    this.xob.onCompleted = function(text) {
        var items = [];
        window.cmdresult = '';
        
        try {
            eval(text);  
            var result = window.cmdresult || '';
            
            if (result.trim()) {
                items = result.trim().split('\n').filter(function(line) {
                    return line.trim();
                });
            }
        } catch(e) {}
        
        self.directoryCache[dir] = items;
        callback(items);
        self.xob = null;
    };
    
    this.xob.onError = function() {
        callback([]);
        self.xob = null;
    };
    
    var cmd = 'ls -1p "' + dir + '" 2>/dev/null';
    
    try {
        var encoded = (typeof escapeCGI === 'function' ? escapeCGI(cmd.replace(/\r/g, '')) : encodeURIComponent(cmd.replace(/\r/g, '')));
        this.xob.post('shell.cgi', 'action=execute&command=' + encoded);
    } catch (e) {
        this.xob = null;
        callback([]);
    }
};

PathIntellisense.prototype.search = function(value) {
    var self = this;
    
    if (!value || !value.trim()) {
        this.hide();
        return;
    }
    
    value = value.replace(/\\/g, '/');
    if (!value.startsWith('/')) value = '/' + value;
    
    var lastSlash = value.lastIndexOf('/');
    var dir = lastSlash === 0 ? '/' : value.substring(0, lastSlash);
    var filter = value.substring(lastSlash + 1).toLowerCase();
    
    this.fetchDirectory(dir, function(items) {
        if (!items || !items.length) {
            self.hide();
            return;
        }
        
        self.suggestions = [];
        
        for (var i = 0; i < items.length; i++) {
            var item = items[i];
            var isDir = item.endsWith('/');
            var cleanName = isDir ? item.slice(0, -1) : item;
            
            if (filter && cleanName.toLowerCase().indexOf(filter) !== 0) continue;
            
            self.suggestions.push({
                name: cleanName,
                isDir: isDir,
                path: dir + (dir.endsWith('/') ? '' : '/') + cleanName + (isDir ? '/' : '')
            });
        }
        
        self.suggestions.sort(function(a, b) {
            return a.name.localeCompare(b.name);
        });
        
        if (self.suggestions.length) {
            self.render();
            self.show();
        } else {
            self.hide();
        }
    });
};

PathIntellisense.prototype.render = function() {
    var self = this;
    var fragment = document.createDocumentFragment();
    this.list.innerHTML = '';
    
    for (var i = 0; i < this.suggestions.length; i++) {
        var item = this.suggestions[i];
        var li = document.createElement('li');
        
        li.textContent = (item.isDir ? '📁 ' : '📄 ') + item.name;
        li.style.cssText = 'padding:10px;cursor:pointer;border-bottom:1px solid #eee;';
        li.setAttribute('data-index', i);
        li.setAttribute('title', item.path);
        
        li.addEventListener('mousedown', function(e) {
            e.preventDefault(); 
            self.select(parseInt(this.getAttribute('data-index'), 10));
        });
        
        li.addEventListener('mouseenter', function() {
            self.highlight(parseInt(this.getAttribute('data-index'), 10));
        });
        
        fragment.appendChild(li);
    }
    
    this.list.appendChild(fragment);
};

PathIntellisense.prototype.show = function() {
    this.container.style.display = 'block';
};

PathIntellisense.prototype.hide = function() {
    this.container.style.display = 'none';
    this.selectedIndex = -1;
};

PathIntellisense.prototype.highlight = function(index) {
    var children = this.list.children;
    for (var i = 0; i < children.length; i++) {
        children[i].style.backgroundColor = '';
    }
    this.selectedIndex = index;
    if (children[index]) {
        children[index].style.backgroundColor = '#f0f0f0';
    }
};

PathIntellisense.prototype.select = function(index) {
    var item = this.suggestions[index];
    this.input.value = item.path;
    this.input.focus();
    this.input.dispatchEvent(new Event('input', { bubbles: true }));
};

PathIntellisense.prototype.handleKeydown = function(e) {
    if (this.container.style.display === 'none') return;
    
    switch(e.keyCode) {
        case 38: 
            e.preventDefault();
            if (this.selectedIndex > 0) this.highlight(this.selectedIndex - 1);
            break;
        case 40: 
            e.preventDefault();
            if (this.selectedIndex < this.suggestions.length - 1) this.highlight(this.selectedIndex + 1);
            break;
        case 13: 
            e.preventDefault();
            if (this.selectedIndex >= 0) this.select(this.selectedIndex);
            break;
        case 27: 
            e.preventDefault();
            this.hide();
            break;
    }
};

function earlyInit() {
    if (typeof PathIntellisense !== 'undefined') {
        new PathIntellisense('pathInput');
    }
}
</script>
</head>

<body>
<table id="container">
<tr><td colspan="2" id="header">
	<div class="title"><a href="/">FreshTomato</a></div>
	<div class="version">Version <% version(); %> on <% nv("t_model_name"); %></div>
</td></tr>
<tr id="body"><td id="navi"><script>navi()</script></td>
<td id="content">
<div id="ident"><% ident(); %> | <script>wikiLink();</script></div>

<div class="section-title">File Browser</div>
<div class="section">
    <div class="form-group" style="margin: 15px 0;">
        <label>Select a file path:</label><br><br>
        <input type="text" id="pathInput" placeholder="Start typing: /mnt, /var, /tmp, /etc..." style="width: 100%; padding: 8px; box-sizing: border-box;">
    </div>
    <div id="debug" style="margin-top: 20px; padding: 10px; background: #f0f0f0; font-family: monospace; font-size: 12px; max-height: 200px; overflow-y: auto;"></div>
</div>

<div id="footer">&nbsp;</div>

</td></tr>
</table>
<script>earlyInit();</script>
</body>
</html>
