import os
import struct
import threading
import queue
import tkinter as tk
from functools import partial
from tkinter import ttk, filedialog, messagebox
from typing import Any, Optional

from PIL import Image, ImageTk, UnidentifiedImageError

Image.MAX_IMAGE_PIXELS = None

SUPPORTED_EXTENSIONS: frozenset[str] = frozenset({
    ".jpg", ".jpeg", ".gif", ".tif", ".tiff", ".bmp", ".png", ".pcx"
})

type ImageRow = dict[str, str | int | None]


def format_bytes(size: int) -> str:
    value = float(size)
    for unit in ("Б", "КБ", "МБ", "ГБ"):
        if value < 1024 or unit == "ГБ":
            return f"{value:.1f} {unit}"
        value /= 1024.0
    return f"{value:.1f} ГБ"


def get_dpi(image: Image.Image) -> tuple[str, str]:
    dpi = image.info.get("dpi")
    if not isinstance(dpi, (tuple, list)) or len(dpi) < 2:
        return "", ""

    try:
        dpi_x = round(float(dpi[0]), 2)
        dpi_y = round(float(dpi[1]), 2)

        if dpi_x == int(dpi_x):
            dpi_x = int(dpi_x)
        if dpi_y == int(dpi_y):
            dpi_y = int(dpi_y)

        return f"{dpi_x} dpi", f"{dpi_y} dpi"
    except (TypeError, ValueError, IndexError):
        return "", ""


def get_compression(image: Image.Image, extension: str) -> str:
    compression = image.info.get("compression")

    if extension in (".jpg", ".jpeg"):
        return "JPEG"

    if extension == ".gif":
        return "LZW"

    if extension == ".png":
        return "Deflate/zlib"

    if extension == ".bmp":
        return str(compression) if compression else "BI_RGB / без сжатия"

    if extension in (".tif", ".tiff"):
        return str(compression) if compression is not None else "Не указано"

    return ""


def get_jpeg_details(image: Image.Image) -> str:
    details: list[str] = []
    quantization: Optional[dict[int, Any]] = getattr(image, "quantization", None)

    if isinstance(quantization, dict) and quantization:
        details.append(f"Таблиц квантования: {len(quantization)}")

        for table_id in sorted(quantization):
            table = quantization[table_id]

            if isinstance(table, (tuple, list)):
                details.append(
                    f"Таблица {table_id}: {len(table)} значений, "
                    f"первые: {list(table[:8])}"
                )
    else:
        details.append("Таблицы квантования не найдены")

    exif = image.getexif()
    if exif:
        details.append(f"EXIF-тегов: {len(exif)}")

    icc_profile = image.info.get("icc_profile")
    if isinstance(icc_profile, (bytes, bytearray)):
        details.append(f"ICC-профиль: {len(icc_profile)} байт")

    return "; ".join(details)


def get_gif_details(image: Image.Image) -> str:
    details: list[str] = []
    palette = image.getpalette()

    if palette is not None:
        details.append(f"Размер глобальной палитры: {len(palette) // 3} цветов")
    else:
        details.append("Глобальная палитра не найдена")

    frames = getattr(image, "n_frames", 1)
    details.append(f"Кадров: {frames}")

    if getattr(image, "is_animated", False):
        details.append("Анимированный GIF")

    transparency = image.info.get("transparency")
    if transparency is not None:
        details.append(f"Прозрачность: индекс {transparency}")

    loop = image.info.get("loop")
    if loop is not None:
        details.append(f"Циклов: {loop if loop != 0 else 'бесконечно'}")

    return "; ".join(details)


def get_tiff_details(image: Image.Image) -> str:
    details: list[str] = []
    frames = getattr(image, "n_frames", 1)
    details.append(f"Страниц/кадров: {frames}")

    for key in ("compression", "photometric", "planar_configuration"):
        value = image.info.get(key)
        if value is not None:
            details.append(f"{key}: {value}")

    if image.info.get("icc_profile") is not None:
        details.append("ICC-профиль присутствует")

    return "; ".join(details)


def get_png_details(image: Image.Image) -> str:
    details: list[str] = []

    if image.mode in ("P", "PA"):
        palette = image.getpalette()
        if palette is not None:
            details.append(f"Палитра: {len(palette) // 3} цветов")

    if image.info.get("transparency") is not None:
        details.append("Прозрачность присутствует")

    if image.info.get("icc_profile") is not None:
        details.append("ICC-профиль присутствует")

    gamma = image.info.get("gamma")
    if gamma is not None:
        details.append(f"Gamma: {gamma}")

    return "; ".join(details)


def get_bmp_details(image: Image.Image) -> str:
    details: list[str] = []

    if image.mode == "P":
        palette = image.getpalette()
        if palette is not None:
            details.append(f"Палитра: {len(palette) // 3} цветов")

    compression = image.info.get("compression")
    if compression:
        details.append(f"Compression: {compression}")

    return "; ".join(details)


def parse_pcx(path: str) -> ImageRow:
    result: ImageRow = {
        "file_name": os.path.basename(path),
        "file_size": "",
        "width": None,
        "height": None,
        "dpi_x": "",
        "dpi_y": "",
        "color_depth": None,
        "compression": "",
        "extra": "",
        "status": "OK",
    }

    try:
        stat = os.stat(path)
        result["file_size"] = format_bytes(stat.st_size)

        with open(path, "rb") as file:
            header = file.read(128)

        if len(header) < 128 or header[0] != 10:
            result["extra"] = "Некорректная сигнатура PCX"
            result["status"] = "Ошибка"
            return result

        (
            _manufacturer,
            version,
            encoding,
            bits_per_plane,
            left, top, right, bottom,
            horizontal_dpi, vertical_dpi,
        ) = struct.unpack_from("<BBBHHHHHHH", header, 0)

        planes: int = header[65]
        width: int = right - left + 1
        height: int = bottom - top + 1
        color_depth: int = bits_per_plane * planes

        if bits_per_plane == 8 and planes == 3:
            color_depth_text = "24 бит/пиксель (RGB)"
        elif bits_per_plane == 8 and planes == 4:
            color_depth_text = "32 бит/пиксель (RGBA)"
        elif bits_per_plane == 8 and planes == 1:
            color_depth_text = "8 бит/пиксель (L или P)"
        elif bits_per_plane == 4 and planes == 1:
            color_depth_text = "4 бит/пиксель (P)"
        elif bits_per_plane == 1 and planes == 1:
            color_depth_text = "1 бит/пиксель (1)"
        else:
            color_depth_text = f"{color_depth} бит/пиксель"

        header_size_valid = (
            0 < width <= 100000
            and 0 < height <= 100000
            and left <= right
            and top <= bottom
        )

        if not header_size_valid:
            try:
                with Image.open(path) as image:
                    width, height = image.size

                result["extra"] = (
                    "Размеры из PCX-заголовка некорректны; "
                    "использованы размеры, определённые Pillow"
                )
            except (OSError, ValueError) as exc:
                result["status"] = "Ошибка"
                result["extra"] = (
                    f"Некорректные размеры в PCX-заголовке; "
                    f"Pillow также не смог открыть файл: {exc}"
                )
                return result

        compression = (
            "RLE" if encoding == 1
            else "Без сжатия" if encoding == 0
            else f"Неизвестный код {encoding}"
        )

        if bits_per_plane == 1 and planes == 1:
            palette_info = "2 цвета (монохром)"
        elif bits_per_plane == 4 and planes == 1:
            palette_info = "16 цветов, палитра в заголовке"
        elif bits_per_plane == 8 and planes == 1:
            palette_info = "до 256 цветов, палитра в конце файла"
        elif bits_per_plane == 8 and planes == 3:
            palette_info = "24-bit True Color"
        elif bits_per_plane == 8 and planes == 4:
            palette_info = "32-bit с альфа-каналом"
        else:
            palette_info = "Нестандартная комбинация бит и плоскостей"

        result.update({
            "width": width,
            "height": height,
            "dpi_x": f"{horizontal_dpi} dpi" if horizontal_dpi else "",
            "dpi_y": f"{vertical_dpi} dpi" if vertical_dpi else "",
            "color_depth": color_depth_text,
            "compression": compression,
            "extra": (
                f"Версия формата: {version}; "
                f"бит/плоскость: {bits_per_plane}; "
                f"плоскостей: {planes}; "
                f"{palette_info}"
            ),
        })

        if not header_size_valid:
            result["extra"] += (
                "; Размеры из PCX-заголовка некорректны; "
                "использованы размеры, определённые Pillow"
            )

    except OSError as exc:
        result["status"] = "Ошибка"
        result["extra"] = f"Ошибка чтения файла: {exc}"
    except struct.error as exc:
        result["status"] = "Ошибка"
        result["extra"] = f"Ошибка разбора PCX-заголовка: {exc}"

    return result

def get_color_depth(image: Image.Image) -> str:
    mode_bits: dict[str, int] = {
        "1": 1,
        "L": 8,
        "P": 8,
        "I": 32,
        "F": 32,
        "RGB": 24,
        "RGBA": 32,
        "CMYK": 32,
        "YCbCr": 24,
        "LAB": 24,
        "HSV": 24,
    }

    bits = mode_bits.get(image.mode)

    if bits is None:
        return f"{image.mode} (глубина не определена)"

    return f"{bits} бит/пиксель ({image.mode})"

def collect_file_info(path: str) -> ImageRow:
    extension = os.path.splitext(path)[1].lower()

    if extension == ".pcx":
        return parse_pcx(path)

    row: ImageRow = {
        "file_name": os.path.basename(path),
        "file_size": "",
        "width": None,
        "height": None,
        "dpi_x": "",
        "dpi_y": "",
        "color_depth": "",
        "compression": "",
        "extra": "",
        "status": "OK",
    }

    try:
        stat = os.stat(path)
        row["file_size"] = format_bytes(stat.st_size)

        with Image.open(path) as image:
            width, height = image.size
            dpi_x, dpi_y = get_dpi(image)

            row.update({
                "width": width,
                "height": height,
                "dpi_x": dpi_x,
                "dpi_y": dpi_y,
                "color_depth": get_color_depth(image),
                "compression": get_compression(image, extension),
            })

            if extension in (".jpg", ".jpeg"):
                row["extra"] = get_jpeg_details(image)
            elif extension == ".gif":
                row["extra"] = get_gif_details(image)
            elif extension in (".tif", ".tiff"):
                row["extra"] = get_tiff_details(image)
            elif extension == ".png":
                row["extra"] = get_png_details(image)
            elif extension == ".bmp":
                row["extra"] = get_bmp_details(image)

    except UnidentifiedImageError:
        row["status"] = "Не распознан"
        row["extra"] = "Pillow не смог определить формат"
    except OSError as exc:
        row["status"] = "Ошибка"
        row["extra"] = f"Ошибка доступа к файлу: {exc}"
    except (ValueError, KeyError) as exc:
        row["status"] = "Ошибка"
        row["extra"] = f"Ошибка обработки изображения: {exc}"

    return row


class ImageInfoApp(tk.Tk):
    def __init__(self) -> None:
        super().__init__()

        self.title("Информация об изображениях")
        self.geometry("1500x750")

        self.folder = tk.StringVar()
        self.recursive = tk.BooleanVar(value=True)

        self.worker: Optional[threading.Thread] = None
        self.stop_event = threading.Event()
        self.result_queue: queue.Queue[tuple[str, Any]] = queue.Queue()
        self.total_files = 0
        self.processed_files = 0

        self.rows_by_item: dict[str, ImageRow] = {}
        self.details_preview_images: dict[str, ImageTk.PhotoImage] = {}

        self.progress: ttk.Progressbar = ttk.Progressbar(self, mode="determinate")
        self.status_label: ttk.Label = ttk.Label(self, text="Готово")
        self.tree: ttk.Treeview = ttk.Treeview(self)

        self.build_ui()
        self.after(100, partial(self.process_queue))

    def build_ui(self) -> None:
        top = ttk.Frame(self, padding=10)
        top.pack(fill="x")

        ttk.Label(top, text="Папка:").pack(side="left")

        ttk.Entry(top, textvariable=self.folder).pack(
            side="left", fill="x", expand=True, padx=8
        )

        ttk.Button(top, text="Выбрать...", command=self.choose_folder).pack(side="left")
        ttk.Button(top, text="Старт", command=self.start_scan).pack(side="left", padx=6)
        ttk.Button(top, text="Стоп", command=self.stop_scan).pack(side="left")

        options = ttk.Frame(self, padding=(10, 0))
        options.pack(fill="x")

        ttk.Checkbutton(
            options,
            text="Включая подпапки",
            variable=self.recursive,
        ).pack(side="left")

        self.progress.pack(fill="x", padx=10, pady=8)
        self.status_label.pack(fill="x", padx=10)

        columns: tuple[str, ...] = (
            "file_name", "file_size", "width", "height",
            "dpi_x", "dpi_y", "color_depth", "compression", "extra", "status",
        )

        headings: dict[str, str] = {
            "file_name": "Имя файла",
            "file_size": "Размер файла",
            "width": "Ширина, px",
            "height": "Высота, px",
            "dpi_x": "DPI по X",
            "dpi_y": "DPI по Y",
            "color_depth": "Глубина цвета",
            "compression": "Сжатие",
            "extra": "Дополнительные данные",
            "status": "Статус",
        }

        table_frame = ttk.Frame(self, padding=10)
        table_frame.pack(fill="both", expand=True)

        self.tree.destroy()
        self.tree = ttk.Treeview(table_frame, columns=columns, show="headings")

        for column in columns:
            anchor: Any = "w" if column in ("file_name", "extra") else "center"

            self.tree.heading(column, text=headings[column])
            self.tree.column(
                column,
                width=260 if column == "extra" else 130,
                anchor=anchor,
                stretch=(column == "extra"),
            )

        vertical_scroll = ttk.Scrollbar(
            table_frame,
            orient="vertical",
            command=self._on_vertical_scroll,
        )

        horizontal_scroll = ttk.Scrollbar(
            table_frame,
            orient="horizontal",
            command=self._on_horizontal_scroll,
        )

        self.tree.configure(
            yscrollcommand=partial(self._sync_vertical_scroll, vertical_scroll),
            xscrollcommand=partial(self._sync_horizontal_scroll, horizontal_scroll),
        )

        self.tree.bind("<Double-1>", self.open_details_window)

        self.tree.grid(row=0, column=0, sticky="nsew")
        vertical_scroll.grid(row=0, column=1, sticky="ns")
        horizontal_scroll.grid(row=1, column=0, sticky="ew")

        table_frame.rowconfigure(0, weight=1)
        table_frame.columnconfigure(0, weight=1)

    def _on_vertical_scroll(self, first: str, _last: str) -> None:
        self.tree.yview_moveto(float(first))

    def _on_horizontal_scroll(self, first: str, _last: str) -> None:
        self.tree.xview_moveto(float(first))

    def _sync_vertical_scroll(self, scrollbar: ttk.Scrollbar, first: str, _last: str) -> None:
        getattr(scrollbar, "set")(first, _last)

    def _sync_horizontal_scroll(self, scrollbar: ttk.Scrollbar, first: str, _last: str) -> None:
        getattr(scrollbar, "set")(first, _last)

    def choose_folder(self) -> None:
        folder = filedialog.askdirectory(title="Выберите папку с изображениями")
        if folder:
            self.folder.set(folder)

    def start_scan(self) -> None:
        folder = self.folder.get().strip()

        if not folder or not os.path.isdir(folder):
            messagebox.showerror("Ошибка", "Укажите существующую папку.")
            return

        worker = self.worker

        if worker is not None and worker.is_alive():
            messagebox.showwarning("Занято", "Сканирование уже выполняется.")
            return

        for item in self.tree.get_children():
            self.tree.delete(item)

        self.rows_by_item.clear()
        self.details_preview_images.clear()

        self.stop_event.clear()
        self.total_files = 0
        self.processed_files = 0
        self.progress.configure(value=0, maximum=100)
        self.status_label.configure(text="Поиск файлов...")

        new_worker = threading.Thread(
            target=self.scan_folder,
            args=(folder, self.recursive.get()),
            daemon=True,
        )

        self.worker = new_worker
        new_worker.start()

    def stop_scan(self) -> None:
        worker = self.worker

        if worker is not None and worker.is_alive():
            self.stop_event.set()
            self.status_label.configure(text="Остановка...")

    def scan_folder(self, folder: str, recursive: bool) -> None:
        files: list[str] = []

        if recursive:
            for root, _dirs, names in os.walk(folder):
                if self.stop_event.is_set():
                    break

                for name in names:
                    extension = os.path.splitext(name)[1].lower()
                    if extension in SUPPORTED_EXTENSIONS:
                        files.append(os.path.join(root, name))
        else:
            for name in os.listdir(folder):
                path = os.path.join(folder, name)
                extension = os.path.splitext(name)[1].lower()

                if os.path.isfile(path) and extension in SUPPORTED_EXTENSIONS:
                    files.append(path)

        self.total_files = len(files)
        self.result_queue.put(("total", self.total_files))

        for path in files:
            if self.stop_event.is_set():
                self.result_queue.put(("stopped", None))
                return

            row = collect_file_info(path)
            row["file_path"] = path

            self.result_queue.put(("row", row))

            self.processed_files += 1
            self.result_queue.put(("progress", self.processed_files))

        self.result_queue.put(("done", None))

    def process_queue(self) -> None:
        try:
            while True:
                kind, value = self.result_queue.get_nowait()

                if kind == "total":
                    if isinstance(value, int) and value > 0:
                        self.status_label.configure(text=f"Найдено файлов: {value}")
                    else:
                        self.status_label.configure(text="Подходящие файлы не найдены")

                elif kind == "row" and isinstance(value, dict):
                    self.append_row(value)

                elif kind == "progress" and isinstance(value, int):
                    if self.total_files:
                        percent = value / self.total_files * 100
                        self.progress.configure(value=percent)

                    self.status_label.configure(
                        text=f"Обработано {value} из {self.total_files}"
                    )

                elif kind == "stopped":
                    self.status_label.configure(text="Сканирование остановлено")

                elif kind == "done":
                    self.progress.configure(value=100)
                    self.status_label.configure(
                        text=f"Готово. Обработано файлов: {self.processed_files}"
                    )

        except queue.Empty:
            pass

        self.after(100, partial(self.process_queue))

    def append_row(self, row: ImageRow) -> None:
        values = [
            str(row.get(key, "") if row.get(key, "") is not None else "")
            for key in (
                "file_name", "file_size", "width", "height",
                "dpi_x", "dpi_y", "color_depth", "compression",
                "extra", "status",
            )
        ]

        item_id = self.tree.insert("", "end", values=values)
        self.rows_by_item[item_id] = row

    def open_details_window(self, event: Any) -> None:
        item_id = self.tree.focus()

        if not item_id or item_id not in self.rows_by_item:
            return

        row = self.rows_by_item[item_id]
        file_path = str(row.get("file_path", ""))

        details_window = tk.Toplevel(self)
        details_window.title(f"Информация: {row.get('file_name', '')}")
        details_window.geometry("720x620")
        details_window.transient(self)
        details_window.grab_set()

        main_frame = ttk.Frame(details_window, padding=16)
        main_frame.pack(fill="both", expand=True)

        preview_frame = ttk.Frame(main_frame)
        preview_frame.pack(fill="x", pady=(0, 12))

        preview_label = ttk.Label(
            preview_frame,
            text="Миниатюра недоступна",
            width=16,
            anchor="center",
        )
        preview_label.pack(side="left", padx=(0, 16))

        self.load_details_preview(details_window, preview_label, file_path)

        title_label = ttk.Label(
            main_frame,
            text=str(row.get("file_name", "")),
            font=("Segoe UI", 12, "bold"),
            wraplength=660,
            justify="left",
        )
        title_label.pack(fill="x", pady=(0, 12))

        fields: tuple[tuple[str, str], ...] = (
            ("Имя файла", "file_name"),
            ("Размер файла", "file_size"),
            ("Ширина", "width"),
            ("Высота", "height"),
            ("DPI по X", "dpi_x"),
            ("DPI по Y", "dpi_y"),
            ("Глубина цвета", "color_depth"),
            ("Сжатие", "compression"),
            ("Дополнительные данные", "extra"),
            ("Статус", "status"),
        )

        for field_name, field_key in fields:
            value = row.get(field_key, "")

            if value is None or value == "":
                value = "Не указано"

            field_frame = ttk.Frame(main_frame)
            field_frame.pack(fill="x", pady=4)

            name_label = ttk.Label(
                field_frame,
                text=f"{field_name}:",
                font=("Segoe UI", 10, "bold"),
                width=24,
                anchor="nw",
            )
            name_label.pack(side="left", fill="x")

            value_label = ttk.Label(
                field_frame,
                text=str(value),
                wraplength=420,
                justify="left",
                anchor="nw",
            )
            value_label.pack(side="left", fill="x", expand=True)

        close_button = ttk.Button(
            main_frame,
            text="Закрыть",
            command=details_window.destroy,
        )
        close_button.pack(pady=(16, 0))

    def load_details_preview(
        self,
        details_window: tk.Toplevel,
        preview_label: ttk.Label,
        file_path: str,
    ) -> None:
        if not file_path or not os.path.isfile(file_path):
            return

        try:
            with Image.open(file_path) as image:
                image.thumbnail((128, 128), Image.Resampling.LANCZOS)

                if image.mode not in ("RGB", "RGBA"):
                    image = image.convert("RGBA")

                photo = ImageTk.PhotoImage(image)
                self.details_preview_images[file_path] = photo

                preview_label.configure(image=photo, text="")
                preview_label.image = photo

        except (OSError, ValueError, UnidentifiedImageError):
            preview_label.configure(text="Миниатюра недоступна")


if __name__ == "__main__":
    app = ImageInfoApp()
    app.mainloop()