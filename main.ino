// Псевдокод
class RobotWaiter
{
private:
	// x координата
	int x;
	// y координата
	int y;
	// По умолчанию робот стоит в левом нижнем углу, но мб что поменяется
	// нулевая x координата
	int x0 = 0;
	// нулевая y координата
	int y0 = 0;
	// скорость перемещения робота [задается мощностью для сервоприводов]
	int speed = 200;
	// начальное направление движения
	int initialDirection = 0;
	// текущие направление движения относительно начального [0 - 270 градусов]
	int direction = 0;
	// минимальная возможная дистанция от робота до стены
	int minPossibleDistance = 0;

	
	// левое заднее
	int wheel1 = 1; 
	// левое переднее
	int wheel2 = 2;
	// правое переднее
	int wheel3 = 3;
	// правое заднее
	int wheel4 = 4;

	// передний дальномер
	int frontRangeFinder = 5;
	// левый дальномер
	int leftRangeFinder = 6;

	// Компас
	int compas = 7;

	// массив точек-углов помещения, из которых строится геометрическая форма помещения
	int corners[100][2];
	int cornersCount = 0;

	// массив пряпятствий, раположенных не по периметру
	int lets[100][2];
	int letsCount = 0;
	
	// массив обычных расположений людей [int id - заказчика => координаты раб. места]
	int peoplesLocations[100][2];
	// массив заказчиков [набор id людей, в базе к id привязывается ip компа и лицо]
	int customers[100];

public:
	/**
	 * движение
	 */
	void go() {
		// подаем мошность на колеса
		for (int i = 0; i < speed; ++i)
		{
			digitalWrire(wheel1, i);
			digitalWrire(wheel2, i);
			digitalWrire(wheel3, i);
			digitalWrire(wheel4, i);
			delay(1);
		}
	}

	/**
	 * Остановка
	 */
	void stop() {
		for (int i = speed; i < 0; ++i)
		{
			digitalWrire(wheel1, i);
			digitalWrire(wheel2, i);
			digitalWrire(wheel3, i);
			digitalWrire(wheel4, i);
			delay(1);
		}
	}
	
	/**
	 *  движение вперед
	 */
	bool goForward() {
		// берем данные с переднего дальномера
		int forwardDistance = getDistanceFromTheFrontRangeFinder();
		// если движение вперед невозможно, сообщаем об этом
		if (forwardDistance <= minPossibleDistance) return false;
		// едем пока расстояние до препятствия не станет минимальным
		while (getDistanceFromTheFrontRangeFinder() > minPossibleDistance) {
			go();
		}
		// отмечаем пройденное расстояние
		setNewYPosition(forwardDistance); 
	}

	/**
	 *  данные с переднего дальномера (свободное пространство впереди)
	 */
	int getDistanceFromTheFrontRangeFinder() {
		return digitalRead(frontRangeFinder);
	}

	/**
	 *  данные с левого дальномера (свободное пространство слева)
	 */
	int getDistanceFromTheLeftRangeFinder() {
		return digitalRead(leftRangeFinder);
	}
	
	/**
	 *  поворот налево
	 */
	void turnLeft() {
		for (int i = 0; i < speed; ++i)
		{
			// на левые колеса отрицательную скорость
			digitalWrire(wheel1, 0-i);
			digitalWrire(wheel2, 0-i);
			// на правые колеса положительную скорость
			digitalWrire(wheel3, i);
			digitalWrire(wheel4, i);
			delay(1);
		}
		
		while(direction <= initialDirection + 90) {
			direction = getDirectionDegreeFormCompas();
		}

		for (int i = speed; i < 0; ++i)
		{
			digitalWrire(wheel1, 0-i);
			digitalWrire(wheel2, 0-i);
			digitalWrire(wheel3, i);
			digitalWrire(wheel4, i);
			delay(1);
		}
	}

	/**
	 *  поворот направо
	 */
	void turnRight() {

		for (int i = 0; i < speed; ++i)
		{
			// на левые колеса положительную скорость
			digitalWrire(wheel1, i);
			digitalWrire(wheel2, i);
			// на правые колеса отрицательную скорость
			digitalWrire(wheel3, 0-i);
			digitalWrire(wheel4, 0-i);
			delay(1);
		}
		
		while(direction <= initialDirection + 90) {
			direction = getDirectionDegreeFormCompas();
		}

		for (int i = speed; i < 0; ++i)
		{
			digitalWrire(wheel1, i);
			digitalWrire(wheel2, i);
			digitalWrire(wheel3, 0-i);
			digitalWrire(wheel4, 0-i);
			delay(1);
		}
	}

	/*
	 * Берем данные с компаса
	 */
	int getDirectionDegreeFormCompas() {
		return digitalRead(compas);
	}

	/**
	 * Выставляем новую позиция робота по y исходя из направления и пройденной дистанции
	 */
	void setNewYPosition(int forwardDistance) {
		// если, направление обратно начальному, т.е. direction = 180,
		if (direction == 180) {
			// то отнимаем от y пройденное расстояние
			y -= forwardDistance;
		} else {
			// иначе прибавляем
			y += forwardDistance;
		}
	}

	void setNewXPosition(int forwardDistance) {
		if (direction == 90) {
			x += forwardDistance;
		} else {
			x -= forwardDistance;
		}
	}

	/**
	 *  протокол движения по периметру
	 */
	void goAlongThePerimetr() {
		go();

		int localX = 0;
		int localY = getDistanceFromTheFrontRangeFinder();

		while (true) {
			// идем прямо, если нет припятствия слева, то поворачиваем налево
			if (getDistanceFromTheLeftRangeFinder() > minPossibleDistance)
			{
				// запоминаем угол
				corners[cornersCount][0] = localX;
				corners[cornersCount][1] = localY;
				cornersCount++;
				
				turnLeft();

				setNewYPosition(y);
				setNewXPosition(x);
			
				localX = getDistanceFromTheFrontRangeFinder();
			}

			// если впереди больше нет места, то поворачиваем направо
			if (getDistanceFromTheFrontRangeFinder() <= minPossibleDistance)
			{
				corners[cornersCount][0] = localX;
				corners[cornersCount][1] = localY;
				cornersCount++;
				
				turnRight();

				setNewYPosition(y);
				setNewXPosition(x);
			
				localX = getDistanceFromTheFrontRangeFinder();
			}
			// если мы на контрольной точке, то останавливаемся
			if (x == x0 && y == y0)
			{
				stop();
				return;
			}

		}
	}

	/**
	 *  протокол обхождения припятствий
	 */
	
	/**
	 *  протокол движения по площади 
	 */
	void goAlongTheSquare() {
		// едем в самую левую точку помещения 
		for (int i = 0; i < corners; ++i)
		{
			/* code */
		}
	}
	
	/**
	 *  протокол движения к конкретной точке в пространстве
	 */
	
	/**
	 *  протокол сканирования лица
	 */
	
	/**
	 *	cохранение всех данных в базу
	 */
	
	/**
	 *	берем данные из базы
	 */
	
};