<!DOCTYPE html>
<!--
	Tomato GUI
	Copyright (C) 2006-2010 Jonathan Zarate
	http://www.polarcloud.com/tomato/
-->
<html lang="en-GB">
<head>
<meta http-equiv="content-type" content="text/html;charset=utf-8">
<meta name="robots" content="noindex,nofollow">
<title>[<% ident(); %>] File Browser</title>
<link rel="stylesheet" type="text/css" href="tomato.css">
<% css(); %>
<style>
    .fm-toolbar { display:flex; gap:10px; margin-bottom:15px; align-items:center; flex-wrap:wrap; position:relative; }
    .fm-btn { padding:4px 10px; cursor:pointer; background:#e0e0e0; border:1px solid #ccc; border-radius:3px; font-size:12px; }
    .fm-btn:hover:not(:disabled) { background:#d0d0d0; }
    .fm-btn-danger { background:#ffcccc; border-color:#ff9999; }
    .fm-btn-danger:hover:not(:disabled) { background:#ffbbbb; }
    .fm-btn-action { background:#ccffcc; border-color:#99ee99; }
    .fm-btn-action:hover:not(:disabled) { background:#bbffbb; }
    .fm-btn:disabled { opacity:0.4 !important; cursor:not-allowed !important; background:#f5f5f5 !important; border-color:#ddd !important; color:#a0a0a0 !important; }
    .fm-table { width:100%; border-collapse:collapse; margin-top:10px; background:#fff; font-size:13px; }
    .fm-table th, .fm-table td { padding:8px; text-align:left; border-bottom:1px solid #eee; word-break:break-all; }
    .fm-table th { background:#f9f9f9; font-weight:bold; }
    .fm-table tr:hover { background:#f5f5f5; }
    .fm-row-dir { cursor:pointer; color:#0055aa; font-weight:bold; }
    .fm-symlink { color:#aa5500; font-style:italic; }
    .fm-modal { display:none; position:fixed; top:0; left:0; width:100%; height:100%; background:rgba(0,0,0,0.6); z-index:10000; align-items:center; justify-content:center; }
    .fm-modal-content { background:white; padding:20px; border-radius:5px; box-shadow:0 4px 15px rgba(0,0,0,0.3); max-height:90vh; overflow-y:auto; }
    .fm-textarea { width:100%; height:400px; font-family:monospace; font-size:13px; margin:10px 0; padding:10px; box-sizing:border-box; resize:vertical; }
    .fm-input { width:100%; padding:8px; box-sizing:border-box; border:1px solid #ccc; border-radius:3px; margin:10px 0; }
    .fm-perm-grid { width:100%; border-collapse:collapse; margin:15px 0; text-align:center; }
    .fm-perm-grid th { background:#eee; padding:5px; border-bottom:1px solid #ddd; }
    .fm-perm-grid td { padding:8px; border-bottom:1px solid #ddd; }
</style>
<script>
//	<% nvram("http_id"); %>
</script>
<script src="tomato.js"></script>
<script>
var FM = {
    currentPath: '/',
    dirWritable: false,
    xob: null,
    actionTarget: '',
    isNewFile: false,
    cmd: function(command, callback) {
        if (this.xob) this.xob = null; 
        this.xob = new XmlHttp();
        this.xob.onCompleted = function(text) {
            var res = '';
            try { eval(text); res = window.cmdresult || ''; } catch(e) {}
            if (callback) callback(res);
            FM.xob = null;
        };
        this.xob.onError = function() {
            if (callback) callback('');
            FM.xob = null;
        };
        try {
            this.xob.post('shell.cgi', 'action=execute&command=' + encodeURIComponent(command.replace(/\r/g, '')));
        } catch (e) {
            if (callback) callback('');
        }
    },
    showAlert: function(msg, showOk) {
        document.getElementById('fm-alert-msg').innerHTML = msg;
        document.getElementById('fm-alert-ok').style.display = showOk ? 'inline-block' : 'none';
        document.getElementById('fm-alert-modal').style.display = 'flex';
    },
    closeAlert: function() { document.getElementById('fm-alert-modal').style.display = 'none'; },
    closeModals: function() {
        var m = document.querySelectorAll('.fm-modal');
        for (var i = 0; i < m.length; i++) m[i].style.display = 'none';
    },
    load: function(dir) {
        dir = (dir ? dir.replace(/\\/g, '/') : '/');
        if (!dir.endsWith('/')) dir += '/';
        this.currentPath = dir;
        var pi = document.getElementById('pathInput');
        if (pi) pi.value = dir;
        var t = 'touch "' + dir + '.rw_test" 2>/dev/null && rm -f "' + dir + '.rw_test" && echo "DIR_WRITABLE=1" || echo "DIR_WRITABLE=0"';
        this.cmd(t + '; ls -lA "' + dir + '" 2>/dev/null', function(res) {
            var lines = res.trim().split('\n');
            FM.dirWritable = (lines.length > 0 && lines[0].indexOf('DIR_WRITABLE=1') === 0);
            lines.shift();
            var btnAdd = document.getElementById('fm-btn-add');
            if (btnAdd) btnAdd.disabled = !FM.dirWritable;
            var btnFolder = document.getElementById('fm-btn-folder');
            if (btnFolder) btnFolder.disabled = !FM.dirWritable;
            FM.renderTable(lines);
        });
    },
    goUp: function() {
        if (this.currentPath === '/') return;
        var p = this.currentPath.split('/').filter(Boolean);
        p.pop();
        this.load('/' + p.join('/'));
    },
    renderTable: function(lines) {
        var tbody = document.getElementById('fm-tbody');
        tbody.innerHTML = '';
        var items = [];
        for (var i = 0; i < lines.length; i++) {
            var line = lines[i].trim();
            if (!line || line.startsWith('total ')) continue;
            var p = line.split(/\s+/);
            if (p.length < 8) continue;
            var perms = p[0], isDir = (perms[0] === 'd'), isSym = (perms[0] === 'l');
            var size = parseInt(p[4], 10), fSize = '-';
            if (!isNaN(size) && !isDir) {
                fSize = size < 1024 ? size + ' B' : (size < 1048576 ? (size/1024).toFixed(1) + ' KB' : (size/1048576).toFixed(2) + ' MB');
            }
            var nameBlock = p.slice(8).join(' '), name = nameBlock, target = '';
            if (isSym) {
                var sp = nameBlock.split(' -> ');
                name = sp[0]; target = sp[1] || '';
            }
            items.push({ name: name, perms: perms, size: fSize, isDir: isDir, isSym: isSym, target: target, fw: FM.dirWritable && perms[2] === 'w' });
        }
        items.sort(function(a, b) { return a.isDir === b.isDir ? a.name.localeCompare(b.name) : (a.isDir ? -1 : 1); });
        if (items.length === 0) {
            tbody.innerHTML = '<tr><td colspan="5" style="text-align:center;color:#888;">Directory is empty</td></tr>';
            return;
        }
        for (var j = 0; j < items.length; j++) {
            var it = items[j], tr = document.createElement('tr');
            var ro = (!it.fw && !it.isDir) || (!FM.dirWritable && it.isDir);
            var badge = ro ? ' <span title="Read-Only">&#128274;</span>' : '';
            var tdName = document.createElement('td');
            if (it.isDir) {
                tdName.className = 'fm-row-dir';
                tdName.innerHTML = '&#128193; ' + it.name + badge;
                tdName.onclick = (function(d) { return function() { FM.load(FM.currentPath + d); }; })(it.name);
            } else {
                tdName.className = it.isSym ? 'fm-symlink' : '';
                tdName.innerHTML = (it.isSym ? '&#128279; ' : '&#128196; ') + it.name + badge;
            }
            tr.appendChild(tdName);
            var tdType = document.createElement('td');
            tdType.innerHTML = it.isSym ? 'Symlink &rarr; ' + it.target : (it.isDir ? 'Directory' : 'File');
            tr.appendChild(tdType);
            var tdSize = document.createElement('td'); tdSize.textContent = it.size; tr.appendChild(tdSize);
            var tdPerms = document.createElement('td'); tdPerms.style.fontFamily = 'monospace'; tdPerms.textContent = it.perms; tr.appendChild(tdPerms);
            var tdAct = document.createElement('td'), bh = '';
            if (!it.isDir) bh += '<button class="fm-btn" onclick="FM.viewFile(\'' + it.name + '\',' + ro + ')">' + (ro ? 'View' : 'Edit') + '</button> ';
            bh += '<button class="fm-btn" ' + (!FM.dirWritable ? 'disabled' : '') + ' onclick="FM.showRename(\'' + it.name + '\')">Rename</button> ';
            bh += '<button class="fm-btn" ' + (!FM.dirWritable ? 'disabled' : '') + ' onclick="FM.showPerms(\'' + it.name + '\',\'' + it.perms + '\')">Perms</button> ';
            bh += '<button class="fm-btn fm-btn-danger" ' + (!FM.dirWritable ? 'disabled' : '') + ' onclick="FM.deleteItem(\'' + it.name + '\')">Delete</button>';
            tdAct.innerHTML = bh;
            tr.appendChild(tdAct);
            tbody.appendChild(tr);
        }
    },
    deleteItem: function(name) {
        if (confirm('Delete: ' + name + '?')) this.cmd('rm -rf "' + this.currentPath + name + '"', function() { FM.load(FM.currentPath); });
    },
    showRename: function(oldName) {
        this.actionTarget = oldName;
        document.getElementById('fm-rename-old').textContent = oldName;
        document.getElementById('fm-rename-input').value = oldName;
        document.getElementById('fm-rename-modal').style.display = 'flex';
        document.getElementById('fm-rename-input').focus();
    },
    execRename: function() {
        var n = document.getElementById('fm-rename-input').value.trim();
        if (!n || n === this.actionTarget) { this.closeModals(); return; }
        this.cmd('mv "' + this.currentPath + this.actionTarget + '" "' + this.currentPath + n + '"', function() {
            FM.closeModals(); FM.load(FM.currentPath);
        });
    },
    showPerms: function(name, pStr) {
        this.actionTarget = name;
        document.getElementById('fm-perms-name').textContent = name;
        var map = ['ur', 'uw', 'ux', 'gr', 'gw', 'gx', 'or', 'ow', 'ox'];
        for (var i = 0; i < 9; i++) {
            var cb = document.getElementById('cb-' + map[i]);
            if (cb) cb.checked = (pStr[i+1] !== '-');
        }
        document.getElementById('fm-perms-modal').style.display = 'flex';
    },
    execPerms: function() {
        var getC = function(id) { return document.getElementById(id).checked ? 1 : 0; };
        var u = getC('cb-ur')*4 + getC('cb-uw')*2 + getC('cb-ux');
        var g = getC('cb-gr')*4 + getC('cb-gw')*2 + getC('cb-gx');
        var o = getC('cb-or')*4 + getC('cb-ow')*2 + getC('cb-ox');
        this.cmd('chmod ' + u + g + o + ' "' + this.currentPath + this.actionTarget + '"', function() {
            FM.closeModals(); FM.load(FM.currentPath);
        });
    },
    openNewFile: function() {
        this.isNewFile = true;
        document.getElementById('fm-modal-title').textContent = 'Create New File in: ' + this.currentPath;
        var ed = document.getElementById('fm-text-editor');
        ed.value = ''; ed.readOnly = false; ed.style.backgroundColor = '#fff';
        document.getElementById('fm-btn-save-editor').style.display = 'inline-block';
        document.getElementById('fm-editor-modal').style.display = 'flex';
    },
    viewFile: function(name, ro) {
        this.isNewFile = false;
        this.actionTarget = name;
        document.getElementById('fm-modal-title').textContent = (ro ? 'Viewing (Read-Only): ' : 'Editing: ') + this.currentPath + name;
        var ed = document.getElementById('fm-text-editor');
        ed.value = 'Loading...'; ed.readOnly = ro; ed.style.backgroundColor = ro ? '#f5f5f5' : '#fff';
        document.getElementById('fm-btn-save-editor').style.display = ro ? 'none' : 'inline-block';
        document.getElementById('fm-editor-modal').style.display = 'flex';
        this.cmd('head -c 500000 "' + this.currentPath + name + '"', function(res) { ed.value = res; });
    },
    saveEditorContent: function() {
        if (this.isNewFile) {
            this.closeModals();
            document.getElementById('fm-newfile-input').value = '';
            document.getElementById('fm-newfile-modal').style.display = 'flex';
            document.getElementById('fm-newfile-input').focus();
        } else {
            this.execWriteFile(this.actionTarget);
        }
    },
    execNewFile: function() {
        var n = document.getElementById('fm-newfile-input').value.trim();
        if (n) this.execWriteFile(n);
    },
    execWriteFile: function(fn) {
        var p = this.currentPath + fn, content = document.getElementById('fm-text-editor').value;
        var ec = content.replace(/\\/g, '\\\\').replace(/\$/g, '\\$').replace(/`/g, '\\`');
        var delim = 'EOF_' + new Date().getTime();
        this.showAlert('Saving...', false);
        this.cmd('cat << \'' + delim + '\' > "' + p + '"\n' + ec + '\n' + delim + '\n', function() {
            FM.closeModals(); FM.showAlert('<b>Success:</b> ' + fn + ' saved!', true); FM.load(FM.currentPath);
        });
    },
    openNewFolder: function() {
        document.getElementById('fm-newfolder-input').value = '';
        document.getElementById('fm-newfolder-modal').style.display = 'flex';
        document.getElementById('fm-newfolder-input').focus();
    },
    execNewFolder: function() {
        var n = document.getElementById('fm-newfolder-input').value.trim();
        if (!n) return;
        var p = this.currentPath + n;
        this.showAlert('Creating folder...', false);
        this.cmd('mkdir -p "' + p + '"', function() {
            FM.closeModals();
            FM.showAlert('<b>Success:</b> Folder ' + n + ' created!', true);
            FM.load(FM.currentPath);
        });
    }
};

var PathIntellisense = function(id) {
    this.input = document.getElementById(id);
    if (!this.input) return;
    this.container = null; this.list = null; this.suggestions = [];
    this.selectedIndex = -1; this.debounceTimer = null; this.directoryCache = {}; this.xob = null;
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
        self.debounceTimer = setTimeout(function() { self.search(self.input.value); }, 300);
    });
    this.input.addEventListener('keydown', function(e) { self.handleKeydown(e); });
    this.input.addEventListener('blur', function() { setTimeout(function() { self.hide(); }, 200); });
};
PathIntellisense.prototype.fetchDirectory = function(dir, cb) {
    var self = this;
    if (this.directoryCache[dir]) { cb(this.directoryCache[dir]); return; }
    if (this.xob) return;
    this.xob = new XmlHttp();
    this.xob.onCompleted = function(text) {
        var items = [];
        window.cmdresult = '';
        try {
            eval(text);
            var res = window.cmdresult || '';
            if (res.trim()) items = res.trim().split('\n').filter(function(l) { return l.trim(); });
        } catch(e) {}
        self.directoryCache[dir] = items;
        cb(items);
        this.xob = null;
    };
    this.xob.onError = function() { cb([]); self.xob = null; };
    try {
        var cmd = 'ls -1p "' + dir + '" 2>/dev/null';
        var encoded = (typeof escapeCGI === 'function' ? escapeCGI(cmd.replace(/\r/g, '')) : encodeURIComponent(cmd.replace(/\r/g, '')));
        this.xob.post('shell.cgi', 'action=execute&command=' + encoded);
    } catch(e) { self.xob = null; cb([]); }
};
PathIntellisense.prototype.search = function(val) {
    var self = this;
    if (!val || !val.trim()) { this.hide(); return; }
    val = val.replace(/\\/g, '/');
    if (!val.startsWith('/')) val = '/' + val;
    var lastS = val.lastIndexOf('/'), dir = lastS === 0 ? '/' : val.substring(0, lastS), filter = val.substring(lastS + 1);
    this.fetchDirectory(dir, function(items) {
        if (!items || !items.length) { self.hide(); return; }
        self.suggestions = [];
        for (var i = 0; i < items.length; i++) {
            var item = items[i], isDir = item.endsWith('/'), clean = isDir ? item.slice(0, -1) : item;
            if (filter && clean.toLowerCase().indexOf(filter.toLowerCase()) !== 0) continue;
            self.suggestions.push({ name: clean, isDir: isDir, path: dir + (dir.endsWith('/') ? '' : '/') + clean + (isDir ? '/' : '') });
        }
        self.suggestions.sort(function(a, b) { return a.name.localeCompare(b.name); });
        if (self.suggestions.length) { self.render(); self.show(); } else { self.hide(); }
    });
};
PathIntellisense.prototype.render = function() {
    var self = this;
    this.list.innerHTML = '';
    for (var i = 0; i < this.suggestions.length; i++) {
        var item = this.suggestions[i], li = document.createElement('li');
        li.innerHTML = (item.isDir ? '&#128193; ' : '&#128196; ') + item.name;
        li.style.cssText = 'padding:8px 10px;cursor:pointer;border-bottom:1px solid #eee;';
        li.setAttribute('data-index', i);
        li.setAttribute('title', item.path);
        li.addEventListener('mousedown', function(e) { e.preventDefault(); self.select(parseInt(this.getAttribute('data-index'))); });
        li.addEventListener('mouseenter', function() { self.highlight(parseInt(this.getAttribute('data-index'))); });
        this.list.appendChild(li);
    }
};
PathIntellisense.prototype.show = function() { this.container.style.display = 'block'; };
PathIntellisense.prototype.hide = function() { this.container.style.display = 'none'; this.selectedIndex = -1; };
PathIntellisense.prototype.highlight = function(idx) {
    for (var i = 0; i < this.list.children.length; i++) this.list.children[i].style.backgroundColor = '';
    this.selectedIndex = idx;
    if (this.list.children[idx]) this.list.children[idx].style.backgroundColor = '#f0f0f0';
};
PathIntellisense.prototype.select = function(idx) {
    var item = this.suggestions[idx];
    this.input.value = item.path;
    this.input.focus();
    this.hide();
    FM.load(item.isDir ? item.path : FM.currentPath);
};
PathIntellisense.prototype.handleKeydown = function(e) {
    if (this.container.style.display === 'none') {
        if (e.keyCode === 13) { e.preventDefault(); FM.load(this.input.value); }
        return;
    }
    switch(e.keyCode) {
        case 38: e.preventDefault(); if (this.selectedIndex > 0) this.highlight(this.selectedIndex - 1); else if (this.selectedIndex === -1 && this.suggestions.length) this.highlight(this.suggestions.length - 1); break;
        case 40: e.preventDefault(); if (this.selectedIndex < this.suggestions.length - 1) this.highlight(this.selectedIndex + 1); else if (this.selectedIndex === -1 && this.suggestions.length) this.highlight(0); break;
        case 13: e.preventDefault(); if (this.selectedIndex >= 0) this.select(this.selectedIndex); else { this.hide(); FM.load(this.input.value); } break;
        case 27: e.preventDefault(); this.hide(); break;
    }
};

function earlyInit() {
    new PathIntellisense('pathInput');
    FM.load('/');
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
<div class="section-title">Web File Manager</div>
<div class="section">
    <div class="fm-toolbar">
        <button class="fm-btn" onclick="FM.goUp()">&#11014; Up</button>
        <button class="fm-btn" onclick="FM.load(FM.currentPath)">&#128260; Refresh</button>
        <input type="text" id="pathInput" style="flex-grow:1; padding:6px; box-sizing:border-box; min-width:150px;" placeholder="/mnt/">
        <button class="fm-btn fm-btn-action" id="fm-btn-add" onclick="FM.openNewFile()">&#10133; New File</button>
        <button class="fm-btn fm-btn-action" id="fm-btn-folder" onclick="FM.openNewFolder()">&#128193; New Folder</button>
    </div>
    <div style="overflow-x:auto;">
        <table class="fm-table">
            <thead>
                <tr>
                    <th style="width:35%;">Name</th>
                    <th style="width:15%;">Type</th>
                    <th style="width:12%;">Size</th>
                    <th style="width:15%;">Perms</th>
                    <th style="width:23%;">Actions</th>
                </tr>
            </thead>
            <tbody id="fm-tbody">
                <tr><td colspan="5">Loading...</td></tr>
            </tbody>
        </table>
    </div>
</div>
<div id="footer">&nbsp;</div>
</td></tr>
</table>

<div id="fm-editor-modal" class="fm-modal">
    <div class="fm-modal-content" style="width:90%; max-width:800px;">
        <h3 id="fm-modal-title" style="margin-top:0;">Edit File</h3>
        <textarea id="fm-text-editor" class="fm-textarea" spellcheck="false"></textarea>
        <div style="text-align:right; margin-top:10px;">
            <button class="fm-btn fm-btn-danger" onclick="FM.closeModals()">Close</button>
            <button class="fm-btn fm-btn-action" id="fm-btn-save-editor" onclick="FM.saveEditorContent()">Save Content</button>
        </div>
    </div>
</div>

<div id="fm-rename-modal" class="fm-modal">
    <div class="fm-modal-content" style="width:90%; max-width:400px;">
        <h3 style="margin-top:0;">Rename Item</h3>
        <p>Enter new name for <b id="fm-rename-old"></b>:</p>
        <input type="text" id="fm-rename-input" class="fm-input" autocomplete="off">
        <div style="text-align:right; margin-top:15px;">
            <button class="fm-btn fm-btn-danger" onclick="FM.closeModals()">Cancel</button>
            <button class="fm-btn fm-btn-action" onclick="FM.execRename()">Save Rename</button>
        </div>
    </div>
</div>

<div id="fm-newfile-modal" class="fm-modal">
    <div class="fm-modal-content" style="width:90%; max-width:400px;">
        <h3 style="margin-top:0;">Name Your File</h3>
        <p>Enter the filename with extension (e.g., script.sh):</p>
        <input type="text" id="fm-newfile-input" class="fm-input" autocomplete="off">
        <div style="text-align:right; margin-top:15px;">
            <button class="fm-btn fm-btn-danger" onclick="FM.closeModals()">Cancel</button>
            <button class="fm-btn fm-btn-action" onclick="FM.execNewFile()">Create File</button>
        </div>
    </div>
</div>

<div id="fm-newfolder-modal" class="fm-modal">
    <div class="fm-modal-content" style="width:90%; max-width:400px;">
        <h3 style="margin-top:0;">Create New Folder</h3>
        <p>Enter the folder name:</p>
        <input type="text" id="fm-newfolder-input" class="fm-input" autocomplete="off">
        <div style="text-align:right; margin-top:15px;">
            <button class="fm-btn fm-btn-danger" onclick="FM.closeModals()">Cancel</button>
            <button class="fm-btn fm-btn-action" onclick="FM.execNewFolder()">Create Folder</button>
        </div>
    </div>
</div>

<div id="fm-perms-modal" class="fm-modal">
    <div class="fm-modal-content" style="width:90%; max-width:400px;">
        <h3 style="margin-top:0;">Change Permissions</h3>
        <p>Set permissions for <b id="fm-perms-name"></b>:</p>
        <table class="fm-perm-grid">
            <tr><th>Target</th><th>Read (r)</th><th>Write (w)</th><th>Execute (x)</th></tr>
            <tr><td><b>Owner</b></td><td><input type="checkbox" id="cb-ur"></td><td><input type="checkbox" id="cb-uw"></td><td><input type="checkbox" id="cb-ux"></td></tr>
            <tr><td><b>Group</b></td><td><input type="checkbox" id="cb-gr"></td><td><input type="checkbox" id="cb-gw"></td><td><input type="checkbox" id="cb-gx"></td></tr>
            <tr><td><b>Public</b></td><td><input type="checkbox" id="cb-or"></td><td><input type="checkbox" id="cb-ow"></td><td><input type="checkbox" id="cb-ox"></td></tr>
        </table>
        <div style="text-align:right; margin-top:15px;">
            <button class="fm-btn fm-btn-danger" onclick="FM.closeModals()">Cancel</button>
            <button class="fm-btn fm-btn-action" onclick="FM.execPerms()">Apply Perms</button>
        </div>
    </div>
</div>

<div id="fm-alert-modal" class="fm-modal" style="z-index:10005;">
    <div class="fm-modal-content" style="width:90%; max-width:350px; text-align:center;">
        <p id="fm-alert-msg" style="margin:15px 0 25px 0; font-size:14px;"></p>
        <button class="fm-btn fm-btn-action" id="fm-alert-ok" onclick="FM.closeAlert()" style="width:100px;">OK</button>
    </div>
</div>

<script>earlyInit();</script>
</body>
</html>