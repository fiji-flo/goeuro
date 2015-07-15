document.onreadystatechange = function () {
    if (document.readyState == "complete") {
        (function() {
            "use strict";
            var input = document.createElement('input');
            input.type = 'text';
            document.body.appendChild(input);
            var button = document.createElement('button');
            button.textContent = 'draw';
            button.addEventListener('click', function () {
                var n = parseInt(input.value, 10);
                n = isNaN(n) ? 0 : n;
                frac(n);
            });
            document.body.appendChild(button);
            var canvas = document.createElement('canvas');
            canvas.width = '200';
            canvas.height = '200';
            document.body.appendChild(canvas);
            var ctx = canvas.getContext('2d');

            function frac(n) {
                ctx.clearRect(0, 0, 200, 200);
                var path = new Path2D();
                path.moveTo(0,100);

                var alpha = 0;
                var x = 0, y = 100;
                var dx = 1, dy = 0;
                function l(deg) {
                    alpha = (alpha - deg) % 360;
                    var bow = Math.PI * (alpha/180);
                    dx = Math.cos(bow);
                    dy = Math.sin(bow);
                }
                function r(deg) {
                    alpha = (alpha + deg) % 360;
                    var bow = Math.PI * (alpha/180);
                    dx = Math.cos(bow);
                    dy = Math.sin(bow);
                }
                function f(len) {
                    x += (dx * len);
                    y += (dy * len);
                    path.lineTo(x, y);
                }
                var segment = (1 /  (2 + Math.sqrt(2)));
                var walk = 200 * Math.pow(segment, n);
                (function draw(it) {
                    if (it === 0) {
                        f(walk);
                    } else {
                        draw(it - 1);
                        l(45);
                        draw(it - 1);
                        r(90);
                        draw(it - 1);
                        l(45);
                        draw(it - 1);
                    }
                })(n);
                ctx.stroke(path);
            }

        })();

    }
};
