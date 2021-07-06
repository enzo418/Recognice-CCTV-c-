
/**
 * Abstract
 */
class ElementController {
    constructor(elm) {
        // if element is null or undefined use an empty div hidden so it doesn't break
        this._element = elm;
        this.initialParent = this._element.parentNode;
    }

    set element(el) {
        this._element = el;
        this.initialParent = el.parentNode;
    }

    get element() {
        return this._element;
    }

    hide() {
        this.element.style.display = "none";
    }

    show() {
        this.element.style.display = "block";
    }

    removeAllChild() {
        this.element.textContent = '';
    }

    scrollContainerToElementTop(container = window) {
        container.scrollTo(0, this.element.offsetTop);
    }

    /**
     * @param {double} width
     */
    set width(width) {
        this.element.style.width = width + "px";
    }

    /**
     * @param {double} height
     */
    set height(height) {
        this.element.style.height = height + "px";
    }

    /**
     * @param {double} opacity
     */
    set opacity(opacity) {
        this.element.style.background = `rgb(0 0 0 / ${opacity}%)`;
    }

    /**
     * @param {HTMLElement} opacity
     */
    move(to) {
        to.appendChild(this.element);
    }

    /**
     * Moves the element to the initial parent
     */
    moveToInitialParent() {
        this.initialParent.appendChild(this.element);
    }

    /**
     * Adds a event listener to the element
     * @param {string} event 
     * @param {function} callback 
     * @param {boolean} useCapture 
     */
    on(event, callback, useCapture = false) {
        this.element.addEventListener(event, callback.bind(this), useCapture);
    }

    /**
     * Sets the element id
     * @param {string} id
     */
    set id(id) {
        this.element.id = id;
        this._id = "#" + id;
    }

    get id() {
        return this.element.id;
    }

    get idSelector() {
        return this._id;
    }

    setAsDraggable(condition_drag_element) {
        var startX = 0,
            startY = 0,
            lastBottom = window.innerHeight * 5 / 100;

        const maxY = window.innerHeight
        const minY = 0

        var maxX = window.innerWidth - this.element.getBoundingClientRect().width
        const minX = 0

        this.element.onmousedown = dragMouseDown.bind(this);

        function dragMouseDown(e) {
            e = e || window.event;
            if (condition_drag_element(e.target)) {
                // recalculate bound
                maxX = window.innerWidth - this.element.getBoundingClientRect().width

                e.preventDefault();
                // get the mouse cursor position at startup:
                startX = e.clientX;
                startY = e.clientY;

                document.onmouseup = closeDragElement.bind(this);
                // call a function whenever the cursor moves:
                document.onmousemove = elementDrag.bind(this);
            }
        }

        function elementDrag(e) {
            e = e || window.event;
            e.preventDefault();

            // move Y
            lastBottom += startY - e.clientY
            lastBottom = lastBottom < minY ? minY : (lastBottom > maxY ? maxY : lastBottom);
            this.element.style.bottom = lastBottom + "px";

            // move X
            var left = this.element.offsetLeft - (startX - e.clientX)
            left = left < minX ? minX : (left >= maxX ? maxX : left);
            this.element.style.left = left + "px";

            startX = e.clientX;
            startY = e.clientY
        }

        function closeDragElement() {
            // stop moving when mouse button is released:
            document.onmouseup = null;
            document.onmousemove = null;
        }
    }
}

export default ElementController;