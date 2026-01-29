"""
drop_manager_server.py

Simple HTTP server for the Drop Manager.
Provides API endpoints to read/write drop data from GameConfigs.db.

Usage: python drop_manager_server.py
Then open http://localhost:8080 in your browser.
"""

import http.server
import socketserver
import json
import sqlite3
from pathlib import Path
from urllib.parse import urlparse, parse_qs
import os

PORT = 8888
DB_PATH = Path(__file__).parent / '../../Binaries/Server/GameConfigs.db'
CHANGELOG_PATH = Path(__file__).parent / 'changelog.txt'

# NPCs that don't have drop tables (guards, war units, special entities)
IGNORED_NPCS = {
    "AGT-Aresden", "AGT-Elvine",       # Town guards
    "CGT-Aresden", "CGT-Elvine",       # City guards
    "MS-Aresden", "MS-Elvine",         # Mages
    "DT-Aresden", "DT-Elvine",         # Defense towers
    "ESG-Aresden", "ESG-Elvine",       # Elite soldiers
    "GMG-Aresden", "GMG-Elvine",       # Grand mages
    "LWB-Aresden", "LWB-Elvine",       # Light war beetles
    "XB-Aresden", "XB-Elvine",         # Summoned creatures
    "XW-Aresden", "XW-Elvine",
    "XY-Aresden", "XY-Elvine",
    "YB-Aresden", "YB-Elvine",
    "YW-Aresden", "YW-Elvine",
    "YY-Aresden", "YY-Elvine",
    "CP-Aresden", "CP-Elvine",         # Catapults
    "Sor-Aresden", "Sor-Elvine",       # Sorcerers
    "ATK-Aresden", "ATK-Evline",       # Attack units
    "Elf-Aresden", "Elf-Elvine",       # Elf guards
    "DSK-Aresden", "DSK-Elvine",       # Dark knights
    "HBT-Aresden", "HBT-Elvine",       # Heldenian battle units
    "CT-Aresden", "CT-Elvine",         # Combat troops
    "Bar-Aresden", "Bar-Elvine",       # Barbarians
    "AGC-Aresden", "AGC-Elvine",       # Archer guards
    "ManaStone",                       # Mana stone (destructible object)
    "GHK", "GHKABS", "TK", "BG"        # Special war NPCs
}


class ReuseAddrTCPServer(socketserver.TCPServer):
    allow_reuse_address = True


class DropManagerHandler(http.server.SimpleHTTPRequestHandler):
    
    def __init__(self, *args, **kwargs):
        # Serve files from tools/Drops directory
        super().__init__(*args, directory=str(Path(__file__).parent), **kwargs)
    
    def do_GET(self):
        parsed = urlparse(self.path)
        
        if parsed.path == '/api/items':
            self.send_json(self.get_items())
        elif parsed.path == '/api/npcs':
            self.send_json(self.get_npcs())
        elif parsed.path == '/api/drops':
            self.send_json(self.get_drops())
        elif parsed.path == '/api/config':
            self.send_json(self.get_config())
        elif parsed.path == '/api/changelog':
            self.send_json(self.get_changelog())
        elif parsed.path == '/':
            self.path = '/drop_manager.html'
            super().do_GET()
        else:
            super().do_GET()
    
    def do_POST(self):
        parsed = urlparse(self.path)
        content_length = int(self.headers.get('Content-Length', 0))
        body = self.rfile.read(content_length).decode('utf-8')
        data = json.loads(body) if body else {}
        
        if parsed.path == '/api/drops/add':
            result = self.add_drop(data)
            self.send_json(result)
        elif parsed.path == '/api/drops/remove':
            result = self.remove_drop(data)
            self.send_json(result)
        elif parsed.path == '/api/drops/update':
            result = self.update_drop(data)
            self.send_json(result)
        elif parsed.path == '/api/settings/update':
            result = self.update_settings(data)
            self.send_json(result)
        elif parsed.path == '/api/changelog/update':
            result = self.update_changelog(data)
            self.send_json(result)
        else:
            self.send_error(404)
    
    def send_json(self, data):
        self.send_response(200)
        self.send_header('Content-Type', 'application/json')
        self.send_header('Access-Control-Allow-Origin', '*')
        self.end_headers()
        self.wfile.write(json.dumps(data).encode('utf-8'))
    
    def get_conn(self):
        return sqlite3.connect(str(DB_PATH.resolve()))
    
    def get_items(self):
        conn = self.get_conn()
        cursor = conn.cursor()
        cursor.execute("SELECT item_id, name FROM items ORDER BY name")
        items = [{"id": row[0], "name": row[1]} for row in cursor.fetchall()]
        conn.close()
        return items
    
    def get_npcs(self):
        conn = self.get_conn()
        cursor = conn.cursor()
        cursor.execute("""
            SELECT dt.drop_table_id, dt.name, dt.description
            FROM drop_tables dt
            ORDER BY dt.drop_table_id
        """)
        npcs = []
        for row in cursor.fetchall():
            # Strip 'drops_' prefix for cleaner display
            name = row[1]
            if name.startswith('drops_'):
                name = name[6:]  # Remove 'drops_' prefix
            # Skip ignored NPCs (guards, war units, etc.)
            if name in IGNORED_NPCS:
                continue
            npcs.append({"id": row[0], "name": name, "description": row[2]})
        conn.close()
        return npcs
    
    def get_drops(self):
        conn = self.get_conn()
        cursor = conn.cursor()
        cursor.execute("""
            SELECT de.drop_table_id, de.tier, de.item_id, de.weight, de.min_count, de.max_count, i.name
            FROM drop_entries de
            LEFT JOIN items i ON de.item_id = i.item_id
            ORDER BY de.drop_table_id, de.tier, de.weight DESC
        """)
        drops = []
        for row in cursor.fetchall():
            item_id = row[2]
            item_name = row[6]
            if item_id == 0:
                item_name = "Nothing"
            elif not item_name:
                item_name = f"Unknown({item_id})"
                
            drops.append({
                "drop_table_id": row[0],
                "tier": row[1],
                "item_id": item_id,
                "weight": row[3],
                "min_count": row[4],
                "max_count": row[5],
                "item_name": item_name
            })
        conn.close()
        return drops
    
    def get_config(self):
        conn = self.get_conn()
        cursor = conn.cursor()
        
        # Get settings
        cursor.execute("SELECT key, value FROM settings")
        settings = {row[0]: row[1] for row in cursor.fetchall()}
        
        conn.close()
        return {
            "settings": settings,
            "global_drops": []
        }
    
    def add_drop(self, data):
        try:
            conn = self.get_conn()
            cursor = conn.cursor()
            cursor.execute("""
                INSERT OR REPLACE INTO drop_entries (drop_table_id, tier, item_id, weight, min_count, max_count)
                VALUES (?, ?, ?, ?, ?, ?)
            """, (data['drop_table_id'], data['tier'], data['item_id'], data['weight'], 
                  data.get('min_count', 1), data.get('max_count', 1)))
            conn.commit()
            
            # Log to changelog
            npc_name = self._get_npc_name(cursor, data['drop_table_id'])
            item_name = self._get_item_name(cursor, data['item_id'])
            conn.close()
            self._log_drop_change(f"Added [{item_name}] to {npc_name} (Tier {data['tier']}, weight {data['weight']})")
            
            return {"success": True}
        except Exception as e:
            return {"success": False, "error": str(e)}
    
    def remove_drop(self, data):
        try:
            conn = self.get_conn()
            cursor = conn.cursor()
            
            # Get names before deleting
            npc_name = self._get_npc_name(cursor, data['drop_table_id'])
            item_name = self._get_item_name(cursor, data['item_id'])
            
            cursor.execute("""
                DELETE FROM drop_entries 
                WHERE drop_table_id = ? AND tier = ? AND item_id = ?
            """, (data['drop_table_id'], data['tier'], data['item_id']))
            conn.commit()
            conn.close()
            
            self._log_drop_change(f"Removed [{item_name}] from {npc_name} (Tier {data['tier']})")
            
            return {"success": True}
        except Exception as e:
            return {"success": False, "error": str(e)}
    
    def update_drop(self, data):
        try:
            conn = self.get_conn()
            cursor = conn.cursor()
            cursor.execute("""
                UPDATE drop_entries 
                SET weight = ?, min_count = ?, max_count = ?
                WHERE drop_table_id = ? AND tier = ? AND item_id = ?
            """, (data['weight'], data.get('min_count', 1), data.get('max_count', 1),
                  data['drop_table_id'], data['tier'], data['item_id']))
            conn.commit()
            conn.close()
            return {"success": True}
        except Exception as e:
            return {"success": False, "error": str(e)}
    
    def update_settings(self, data):
        """Update settings in the database and log to changelog"""
        try:
            conn = self.get_conn()
            cursor = conn.cursor()
            
            changes = []
            for key, value in data.items():
                # Get old value
                cursor.execute("SELECT value FROM settings WHERE key = ?", (key,))
                row = cursor.fetchone()
                old_value = row[0] if row else 'N/A'
                
                # Update or insert
                cursor.execute("INSERT OR REPLACE INTO settings (key, value) VALUES (?, ?)", (key, str(value)))
                changes.append(f"  {key}: {old_value} → {value}")
            
            conn.commit()
            conn.close()
            
            # Auto-log to changelog
            if changes:
                from datetime import datetime
                timestamp = datetime.now().strftime("%Y-%m-%d %H:%M:%S")
                entry = f"[{timestamp}] Settings updated:\n" + "\n".join(changes) + "\n\n"
                self._append_changelog(entry)
            
            return {"success": True}
        except Exception as e:
            return {"success": False, "error": str(e)}
    
    def get_changelog(self):
        """Read changelog from file"""
        try:
            if CHANGELOG_PATH.exists():
                return {"content": CHANGELOG_PATH.read_text(encoding='utf-8')}
            return {"content": ""}
        except Exception as e:
            return {"content": "", "error": str(e)}
    
    def update_changelog(self, data):
        """Write changelog to file"""
        try:
            CHANGELOG_PATH.write_text(data.get('content', ''), encoding='utf-8')
            return {"success": True}
        except Exception as e:
            return {"success": False, "error": str(e)}
    
    def _append_changelog(self, entry):
        """Append entry to changelog file"""
        try:
            existing = ""
            if CHANGELOG_PATH.exists():
                existing = CHANGELOG_PATH.read_text(encoding='utf-8')
            CHANGELOG_PATH.write_text(entry + existing, encoding='utf-8')
        except Exception:
            pass
    
    def _get_npc_name(self, cursor, drop_table_id):
        """Get NPC/drop table name by ID"""
        cursor.execute("SELECT name FROM drop_tables WHERE drop_table_id = ?", (drop_table_id,))
        row = cursor.fetchone()
        if row:
            name = row[0]
            if name.startswith('drops_'):
                name = name[6:]
            return name
        return f"Table#{drop_table_id}"
    
    def _get_item_name(self, cursor, item_id):
        """Get item name by ID"""
        if item_id == 0:
            return "Nothing"
        cursor.execute("SELECT name FROM items WHERE item_id = ?", (item_id,))
        row = cursor.fetchone()
        return row[0] if row else f"Item#{item_id}"
    
    def _log_drop_change(self, message):
        """Log a drop change with timestamp"""
        from datetime import datetime
        timestamp = datetime.now().strftime("%Y-%m-%d %H:%M:%S")
        entry = f"[{timestamp}] {message}\n"
        self._append_changelog(entry)


def main():
    os.chdir(Path(__file__).parent)
    
    with ReuseAddrTCPServer(("", PORT), DropManagerHandler) as httpd:
        print(f"🎮 Drop Manager running at http://localhost:{PORT}")
        print("Press Ctrl+C to stop")
        try:
            httpd.serve_forever()
        except KeyboardInterrupt:
            print("\n👋 Server stopped")


if __name__ == "__main__":
    main()
