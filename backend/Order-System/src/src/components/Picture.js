import React, { useState } from "react";
import { Card, Button, Form } from "react-bootstrap";

const Picture = ({ data, name, price, index, addHandler, totalPriceHandler }) => {
  const [selectValue, setSelectValue] = useState(0);

  const handleChange = (e) => {
    setSelectValue(!!+e.target.value ? +e.target.value : 0);
  };

  const addHandlerCaller = () => {
    addHandler(index, selectValue);
    totalPriceHandler(selectValue * price);
    setSelectValue(0);
  };

  return (
    <div className="picture">
      <Card style={{ width: "18rem" }}>
        <Card.Img variant="top" src={require(`../assets/top-food-${data}.jpg`)} />
        <Card.Body>
          <Card.Title>{name}</Card.Title>
          <Card.Text>
            Some quick example text to build on the card title and make up the bulk of the card's content.
          </Card.Text>
          <Card.Text>Price : NT$ {price}</Card.Text>
          <Form.Select aria-label="Default select example" onChange={handleChange} value={selectValue}>
            <option>Choose Amount</option>
            <option value="1">1</option>
            <option value="2">2</option>
            <option value="3">3</option>
            <option value="4">4</option>
            <option value="5">5</option>
            <option value="6">6</option>
            <option value="7">7</option>
            <option value="8">8</option>
            <option value="9">9</option>
            <option value="10">10</option>
          </Form.Select>

          <Button variant="outline-primary" style={{ width: "60%" }} onClick={addHandlerCaller}>
            Add
          </Button>
        </Card.Body>
      </Card>
    </div>
  );
};

export default Picture;
